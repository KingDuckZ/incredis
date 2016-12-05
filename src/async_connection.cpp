/* Copyright 2016, Michele Santullo
 * This file is part of "incredis".
 *
 * "incredis" is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * "incredis" is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with "incredis".  If not, see <http://www.gnu.org/licenses/>.
 */

#include "async_connection.hpp"
#include <hiredis/async.h>
#include <hiredis/adapters/libev.h>
#include <ev.h>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <mutex>
#include <signal.h>
#include <cassert>
#include <sstream>

namespace redis {
	namespace {
		void async_callback (ev_loop* /*parLoop*/, ev_async* /*parObject*/, int /*parRevents*/) {
		}

		void async_halt_loop (ev_loop* parLoop, ev_async* /*parObject*/, int /*parRevents*/) {
			ev_break(parLoop, EVBREAK_ALL);
		}

		void lock_mutex_libev (ev_loop* parLoop) noexcept {
			std::mutex* mtx = static_cast<std::mutex*>(ev_userdata(parLoop));
			assert(mtx);
			try {
				mtx->lock();
			}
			catch (const std::system_error&) {
				assert(false);
			}
		}

		void unlock_mutex_libev (ev_loop* parLoop) noexcept {
			std::mutex* mtx = static_cast<std::mutex*>(ev_userdata(parLoop));
			assert(mtx);
			mtx->unlock();
		}
	} //unnamed namespace

	struct AsyncConnection::LocalData {
		LocalData() :
			redis_poll_thread(),
			connect_processed(false),
			disconnect_processed(true)
		{
		}

		ev_async watcher_wakeup;
		ev_async watcher_halt;
		std::thread redis_poll_thread;
		std::mutex hiredis_mutex;
		std::mutex libev_mutex;
		std::condition_variable condition_connected;
		std::condition_variable condition_disconnected;
		std::string connect_err_msg;
		std::atomic_bool connect_processed;
		std::atomic_bool disconnect_processed;
	};

	void on_connect (const redisAsyncContext* parContext, int parStatus) {
		assert(parContext and parContext->data);
		AsyncConnection& self = *static_cast<AsyncConnection*>(parContext->data);
		assert(parContext == self.m_conn.get());
		assert(not self.m_local_data->connect_processed);

		self.m_connection_lost = false;
		self.m_connected = (parStatus == REDIS_OK);
		self.m_local_data->connect_processed = true;
		self.m_local_data->connect_err_msg = parContext->errstr;
		self.m_local_data->condition_connected.notify_one();
	}

	void on_disconnect (const redisAsyncContext* parContext, int parStatus) {
		assert(parContext and parContext->data);
		AsyncConnection& self = *static_cast<AsyncConnection*>(parContext->data);
		assert(self.m_connected);
		assert(not self.m_local_data->disconnect_processed);

		self.m_connection_lost = (REDIS_ERR == parStatus);
		self.m_connected = false;
		self.m_local_data->disconnect_processed = true;
		self.m_local_data->connect_err_msg.clear();
		self.m_local_data->condition_disconnected.notify_one();
	};

	AsyncConnection::AsyncConnection(std::string&& parAddress, uint16_t parPort) :
		m_conn(nullptr, &redisAsyncDisconnect),
		m_local_data(new LocalData()),
		m_libev_loop_thread(ev_loop_new(EVFLAG_NOINOTIFY), &ev_loop_destroy),
		m_address(std::move(parAddress)),
		m_port(parPort),
		m_connected(false),
		m_connection_lost(false)
	{
		//Init libev stuff
		{
			signal(SIGPIPE, SIG_IGN);

			//See: http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#THREAD_LOCKING_EXAMPLE
			ev_async_init(&m_local_data->watcher_wakeup, &async_callback);
			ev_async_start(m_libev_loop_thread.get(), &m_local_data->watcher_wakeup);
			ev_async_init(&m_local_data->watcher_halt, &async_halt_loop);
			ev_async_start(m_libev_loop_thread.get(), &m_local_data->watcher_halt);
			ev_set_userdata(m_libev_loop_thread.get(), &m_local_data->libev_mutex);
			ev_set_loop_release_cb(m_libev_loop_thread.get(), &unlock_mutex_libev, &lock_mutex_libev);
		}
	}

	AsyncConnection::~AsyncConnection() noexcept {
		this->disconnect();
		this->wait_for_disconnect();
	}

	void AsyncConnection::connect() {
		if (not m_conn) {
			m_local_data->disconnect_processed = false;
			RedisConnection conn(
				(is_socket_connection() ?
					redisAsyncConnectUnix(m_address.c_str())
				:
					redisAsyncConnect(m_address.c_str(), m_port)
				),
				&redisAsyncDisconnect
			);
			if (not conn) {
				std::ostringstream oss;
				oss << "Unable to connect to Redis server at " << m_address << ':' << m_port;
				throw std::runtime_error(oss.str());
			}
			else {
				conn->data = this;
			}
			if (REDIS_OK != redisLibevAttach(m_libev_loop_thread.get(), conn.get()))
				throw std::runtime_error("Unable to set event loop");
			if (REDIS_OK != redisAsyncSetConnectCallback(conn.get(), &on_connect))
				throw std::runtime_error("Unable to set \"on_connect()\" callback");
			if (REDIS_OK != redisAsyncSetDisconnectCallback(conn.get(), &on_disconnect))
				throw std::runtime_error("Unable to set \"on_disconnect()\" callback");
			std::swap(conn, m_conn);
			m_local_data->redis_poll_thread = std::thread([this]() {
				m_local_data->libev_mutex.lock();
				ev_run(m_libev_loop_thread.get(), 0);
				m_local_data->libev_mutex.unlock();
			});
			wakeup_event_thread();
		}
	}

	void AsyncConnection::wait_for_connect() {
		if (not m_local_data->connect_processed) {
			std::unique_lock<std::mutex> lk(m_local_data->hiredis_mutex);
			m_local_data->condition_connected.wait(lk, [this]() { return m_local_data->connect_processed.load(); });
			assert(true == m_local_data->connect_processed);
		}
	}

	void AsyncConnection::disconnect() {
		if (not m_local_data->connect_processed)
			return;
		assert(m_local_data->redis_poll_thread.joinable());
		m_local_data->connect_processed = false;
		{
			std::lock_guard<std::mutex> lock(m_local_data->libev_mutex);
			assert(not ev_async_pending(&m_local_data->watcher_halt));
			ev_async_send(m_libev_loop_thread.get(), &m_local_data->watcher_halt);
			m_conn.reset();
		}
		m_local_data->redis_poll_thread.join();
	}

	void AsyncConnection::wait_for_disconnect() {
		if (not m_local_data->disconnect_processed) {
			std::unique_lock<std::mutex> lk(m_local_data->hiredis_mutex);
			m_local_data->condition_disconnected.wait(lk, [this]() { return m_local_data->disconnect_processed.load(); });
			assert(true == m_local_data->disconnect_processed);
		}
	}

	bool AsyncConnection::is_connected() const {
		const bool connected = m_conn and not m_conn->err and m_connected;
		assert(not m_connection_lost or connected);
		return connected;
	}

	boost::string_ref AsyncConnection::connection_error() const {
		return m_local_data->connect_err_msg;
	}

	void AsyncConnection::wakeup_event_thread() {
		if (ev_async_pending(&m_local_data->watcher_wakeup) == false) {
			std::lock_guard<std::mutex> lock(m_local_data->libev_mutex);
			ev_async_send(m_libev_loop_thread.get(), &m_local_data->watcher_wakeup);
		}
	}

	std::mutex& AsyncConnection::event_mutex() {
		return m_local_data->libev_mutex;
	}

	bool AsyncConnection::is_socket_connection() const {
		return not (m_port or m_address.empty());
	}
} //namespace redis
