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

#ifndef id982A651A9BC34F6E9BA935A804B3A0A4
#define id982A651A9BC34F6E9BA935A804B3A0A4

#include <memory>
#include <string>
#include <cstdint>
#include <boost/utility/string_ref.hpp>

struct redisAsyncContext;
struct ev_loop;

namespace std {
	class mutex;
} //namespace std

namespace redis {
	class AsyncConnection {
		friend void on_connect ( const redisAsyncContext*, int );
		friend void on_disconnect ( const redisAsyncContext*, int );
	public:
		AsyncConnection ( std::string&& parAddress, uint16_t parPort );
		~AsyncConnection ( void ) noexcept;

		void connect ( void );
		void wait_for_connect ( void );
		void disconnect ( void );
		void wait_for_disconnect ( void );

		bool is_connected ( void ) const;
		boost::string_ref connection_error ( void ) const;
		void wakeup_event_thread ( void );
		std::mutex& event_mutex ( void );
		redisAsyncContext* connection ( void );

	private:
		using RedisConnection = std::unique_ptr<redisAsyncContext, void(*)(redisAsyncContext*)>;
		using LibevLoop = std::unique_ptr<ev_loop, void(*)(ev_loop*)>;

		bool is_socket_connection ( void ) const;

		struct LocalData;

		RedisConnection m_conn;
		std::unique_ptr<LocalData> m_local_data;
		LibevLoop m_libev_loop_thread;
		std::string m_address;
		uint16_t m_port;
		volatile bool m_connected;
		volatile bool m_connection_lost;
	};

	inline redisAsyncContext* AsyncConnection::connection() {
		return m_conn.get();
	}
} //namespace redis

#endif
