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

#ifndef id7D338900114548A890B1EECE0C4D3C4C
#define id7D338900114548A890B1EECE0C4D3C4C

#include "command.hpp"
#include "incredis_batch.hpp"
#include "scan_iterator.hpp"
#include <boost/optional.hpp>
#include <string>
#include <boost/utility/string_view.hpp>
#include <vector>
#include <boost/range/iterator_range_core.hpp>
#include <boost/range/empty.hpp>
#include <utility>

namespace redis {
	class IncRedis {
	public:
		typedef ScanIterator<ScanSingleValues<std::string>> scan_iterator;
		typedef boost::iterator_range<scan_iterator> scan_range;
		typedef ScanIterator<ScanPairs<std::pair<std::string, std::string>, ScanCommands::HSCAN>> hscan_iterator;
		typedef boost::iterator_range<hscan_iterator> hscan_range;
		typedef ScanIterator<ScanSingleValuesInKey<std::string>> sscan_iterator;
		typedef boost::iterator_range<sscan_iterator> sscan_range;
		typedef ScanIterator<ScanPairs<std::pair<std::string, std::string>, ScanCommands::ZSCAN>> zscan_iterator;
		typedef boost::iterator_range<zscan_iterator> zscan_range;

		typedef boost::optional<std::string> opt_string;
		typedef boost::optional<std::vector<opt_string>> opt_string_list;

		IncRedis ( std::string&& parAddress, uint16_t parPort );
		IncRedis ( IncRedis&& ) = default;
		explicit IncRedis ( std::string&& parSocket );
		~IncRedis ( void ) noexcept = default;

		void connect ( void );
		void wait_for_connect ( void );
		void disconnect ( void );
		void wait_for_disconnect ( void );
		bool is_connected ( void ) const { return m_command.is_connected(); }

		IncRedisBatch make_batch ( void );

		Command& command ( void ) { return m_command; }
		const Command& command ( void ) const { return m_command; }

		//Scan
		scan_range scan ( boost::string_view parPattern=boost::string_view() );
		hscan_range hscan ( boost::string_view parKey, boost::string_view parPattern=boost::string_view() );
		sscan_range sscan ( boost::string_view parKey, boost::string_view parPattern=boost::string_view() );
		zscan_range zscan ( boost::string_view parKey, boost::string_view parPattern=boost::string_view() );

		//Hash
		opt_string hget ( boost::string_view parKey, boost::string_view parField );
		template <typename... Args>
		opt_string_list hmget ( boost::string_view parKey, Args&&... parArgs );
		template <typename... Args>
		bool hmset ( boost::string_view parKey, Args&&... parArgs );
		RedisInt hincrby ( boost::string_view parKey, boost::string_view parField, int parInc );

		//Set
		opt_string_list srandmember ( boost::string_view parKey, int parCount );
		opt_string srandmember ( boost::string_view parKey );
		opt_string_list smembers ( boost::string_view parKey );

		//Sorted set
		opt_string_list zrangebyscore ( boost::string_view parKey, double parMin, bool parMinIncl, double parMax, bool parMaxIncl, bool parWithScores );

		//Script
		bool script_flush ( void );

		//Misc
		bool flushdb ( void );
		RedisInt dbsize ( void );
		bool expire ( boost::string_view parKey, RedisInt parTTL );
		template <typename... Args>
		RedisInt del (Args&&... parArgs);

		//String
		opt_string get ( boost::string_view parKey );
		bool set ( boost::string_view parKey, boost::string_view parField );
		RedisInt incr ( boost::string_view parKey );

	private:
		static opt_string_list reply_to_string_list ( const Reply& parReply );

		Command m_command;
	};

	template <typename... Args>
	auto IncRedis::hmget (boost::string_view parKey, Args&&... parArgs) -> opt_string_list {
		static_assert(sizeof...(Args) > 0, "No fields specified");
		return reply_to_string_list(m_command.run("HMGET", parKey, std::forward<Args>(parArgs)...));
	}

	template <typename... Args>
	bool IncRedis::hmset (boost::string_view parKey, Args&&... parArgs) {
		static_assert(sizeof...(Args) > 0, "No fields specified");
		static_assert(sizeof...(Args) % 2 == 0, "Uneven number of parameters received");
		const auto ret = redis::get<StatusString>(m_command.run("HMSET", parKey, std::forward<Args>(parArgs)...));
		return ret.is_ok();
	}

	template <typename... Args>
	RedisInt IncRedis::del (Args&&... parArgs) {
		static_assert(sizeof...(Args) > 0, "No keys specified");
		const auto ret = m_command.run("DEL", std::forward<Args>(parArgs)...);
		return get_integer(ret);
	}

} //namespace redis

#endif
