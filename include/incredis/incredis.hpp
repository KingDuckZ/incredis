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
#include <boost/utility/string_ref.hpp>
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
		scan_range scan ( boost::string_ref parPattern=boost::string_ref() );
		hscan_range hscan ( boost::string_ref parKey, boost::string_ref parPattern=boost::string_ref() );
		sscan_range sscan ( boost::string_ref parKey, boost::string_ref parPattern=boost::string_ref() );
		zscan_range zscan ( boost::string_ref parKey, boost::string_ref parPattern=boost::string_ref() );

		//Hash
		opt_string hget ( boost::string_ref parKey, boost::string_ref parField );
		template <typename... Args>
		opt_string_list hmget ( boost::string_ref parKey, Args&&... parArgs );
		int hincrby ( boost::string_ref parKey, boost::string_ref parField, int parInc );

		//Set
		opt_string_list srandmember ( boost::string_ref parKey, int parCount );
		opt_string srandmember ( boost::string_ref parKey );
		opt_string_list smembers ( boost::string_ref parKey );

		//Sorted set
		opt_string_list zrangebyscore ( boost::string_ref parKey, double parMin, bool parMinIncl, double parMax, bool parMaxIncl, bool parWithScores );

		//Script
		bool script_flush ( void );

		//Misc
		bool flushdb ( void );
		RedisInt dbsize ( void );

		//String
		opt_string get ( boost::string_ref parKey );
		bool set ( boost::string_ref parKey, boost::string_ref parField );
		RedisInt incr ( boost::string_ref parKey );

	private:
		static opt_string_list reply_to_string_list ( const Reply& parReply );

		Command m_command;
	};

	template <typename... Args>
	auto IncRedis::hmget (boost::string_ref parKey, Args&&... parArgs) -> opt_string_list {
		static_assert(sizeof...(Args) > 0, "No fields specified");
		return reply_to_string_list(m_command.run("HMGET", parKey, std::forward<Args>(parArgs)...));
	}
} //namespace redis

#endif
