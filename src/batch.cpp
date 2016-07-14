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

#include "batch.hpp"
#include "async_connection.hpp"
#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include <cassert>
#include <future>
#include <ciso646>
#include <boost/iterator/transform_iterator.hpp>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <sstream>

//#define VERBOSE_HIREDIS_COMM

#if defined(VERBOSE_HIREDIS_COMM)
#   include <iostream>
#endif

namespace redis {
	namespace {
		const std::size_t g_max_redis_unanswered_commands = 1000;

		struct HiredisCallbackData {
			HiredisCallbackData ( std::atomic_size_t& parPendingFutures, std::condition_variable& parSendCmdCond ) :
				promise(),
				pending_futures(parPendingFutures),
				send_command_condition(parSendCmdCond)
			{
			}

			std::promise<Reply> promise;
			std::atomic_size_t& pending_futures;
			std::condition_variable& send_command_condition;
		};

		Reply make_redis_reply_type (redisReply* parReply) {
			using boost::transform_iterator;
			using PtrToReplyIterator = transform_iterator<Reply(*)(redisReply*), redisReply**>;

			switch (parReply->type) {
			case REDIS_REPLY_INTEGER:
				return parReply->integer;
			case REDIS_REPLY_STRING:
				return std::string(parReply->str, parReply->len);
			case REDIS_REPLY_ARRAY:
				return std::vector<Reply>(
					PtrToReplyIterator(parReply->element, &make_redis_reply_type),
					PtrToReplyIterator(parReply->element + parReply->elements, &make_redis_reply_type)
				);
			case REDIS_REPLY_ERROR:
				return ErrorString(parReply->str, parReply->len);
			case REDIS_REPLY_STATUS:
				return StatusString(parReply->str, parReply->len);
			case REDIS_REPLY_NIL:
				return nullptr;
			default:
				assert(false); //not reached
				return Reply();
			};
		}

		void hiredis_run_callback (redisAsyncContext*, void* parReply, void* parPrivData) {
			assert(parPrivData);
			auto* data = static_cast<HiredisCallbackData*>(parPrivData);
			const auto old_count = data->pending_futures.fetch_add(-1);
			assert(old_count > 0);
			if (old_count == g_max_redis_unanswered_commands)
				data->send_command_condition.notify_one();

			if (parReply) {
				auto reply = make_redis_reply_type(static_cast<redisReply*>(parReply));
				data->promise.set_value(std::move(reply));
			}
			else {
				assert(false); //Should this case also be managed?
			}

			delete data;
		}

		int array_throw_if_failed (int parErrCount, int parMaxReportedErrors, const std::vector<Reply>& parReplies, std::ostream& parStream) {
			int err_count = 0;
			for (const auto& rep : parReplies) {
				if (rep.which() == RedisVariantType_Error) {
					++err_count;
					if (err_count + parErrCount <= parMaxReportedErrors)
						parStream << '"' << get_error_string(rep).message() << "\" ";
				}
				else if (rep.which() == RedisVariantType_Array) {
					err_count += array_throw_if_failed(err_count + parErrCount, parMaxReportedErrors, get_array(rep), parStream);
				}
			}
			return err_count;
		}
	} //unnamed namespace

	struct Batch::LocalData {
		explicit LocalData ( std::atomic_size_t& parPendingFutures ) :
			free_cmd_slot(),
			futures_mutex(),
			pending_futures(parPendingFutures)
		{
		}

		std::condition_variable free_cmd_slot;
		std::mutex futures_mutex;
		std::atomic_size_t& pending_futures;
	};

	Batch::Batch (Batch&&) = default;

	Batch::Batch (AsyncConnection* parConn, std::atomic_size_t& parPendingFutures) :
		m_futures(),
		m_replies(),
		m_local_data(new LocalData(parPendingFutures)),
		m_async_conn(parConn)
	{
		assert(m_async_conn);
		assert(m_async_conn->connection());
	}

	Batch::~Batch() noexcept {
		if (m_local_data)
			this->reset();
	}

	void Batch::run_pvt (int parArgc, const char** parArgv, std::size_t* parLengths) {
		assert(not replies_requested());
		assert(parArgc >= 1);
		assert(parArgv);
		assert(parLengths); //This /could/ be null, but I don't see why it should
		assert(m_local_data);

		const auto pending_futures = m_local_data->pending_futures.fetch_add(1);
		auto* data = new HiredisCallbackData(m_local_data->pending_futures, m_local_data->free_cmd_slot);

#if defined(VERBOSE_HIREDIS_COMM)
		std::cout << "run_pvt(), " << pending_futures << " items pending... ";
#endif
		if (pending_futures >= g_max_redis_unanswered_commands) {
#if defined(VERBOSE_HIREDIS_COMM)
			std::cout << " waiting... ";
#endif
			std::unique_lock<std::mutex> u_lock(m_local_data->futures_mutex);
			m_local_data->free_cmd_slot.wait(u_lock, [this]() { return m_local_data->pending_futures < g_max_redis_unanswered_commands; });
		}
#if defined(VERBOSE_HIREDIS_COMM)
		std::cout << " emplace_back(future)... ";
#endif

		m_futures.emplace_back(data->promise.get_future());
		{
			std::lock_guard<std::mutex> lock(m_async_conn->event_mutex());
			const int command_added = redisAsyncCommandArgv(m_async_conn->connection(), &hiredis_run_callback, data, parArgc, parArgv, parLengths);
			assert(REDIS_OK == command_added); // REDIS_ERR if error
			static_cast<void>(command_added);
		}

#if defined(VERBOSE_HIREDIS_COMM)
		std::cout << "command sent to hiredis" << std::endl;
#endif
		m_async_conn->wakeup_event_thread();
	}

	bool Batch::replies_requested() const {
		return not m_replies.empty();
	}

	const std::vector<Reply>& Batch::replies() {
		if (not replies_requested()) {
			m_replies.reserve(m_futures.size());
			for (auto& fut : m_futures) {
				m_replies.emplace_back(fut.get());
			}

			auto empty_vec = std::move(m_futures);
		}
		return m_replies;
	}

	void Batch::throw_if_failed() {
		std::ostringstream oss;
		const int max_reported_errors = 3;

		oss << "Error in reply: ";
		const int err_count = array_throw_if_failed(0, max_reported_errors, replies(), oss);
		if (err_count) {
			oss << " (showing " << err_count << '/' << max_reported_errors << " errors on " << replies().size() << " total replies)";
			throw std::runtime_error(oss.str());
		}
	}

	void Batch::reset() noexcept {
		try {
			this->replies(); //force waiting for any pending jobs
		}
		catch (...) {
			assert(false);
		}

		assert(m_local_data);
		assert(0 == m_local_data->pending_futures);
		m_futures.clear();
		m_replies.clear();
	}

	RedisError::RedisError (const char* parMessage, std::size_t parLength) :
		std::runtime_error(std::string(parMessage, parLength))
	{
	}
} //namespace redis
