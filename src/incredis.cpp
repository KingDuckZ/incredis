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

#include "incredis.hpp"
#include "duckhandy/compatibility.h"
#include "incredis/int_conv.hpp"
#include <cassert>
#include <ciso646>

namespace redis {
	namespace {
		inline IncRedis::opt_string optional_string ( const Reply& parReply ) a_always_inline;
		IncRedis::opt_string_list optional_string_list ( const Reply& parReply );

		IncRedis::opt_string optional_string (const Reply& parReply) {
			assert(parReply.which() == RedisVariantType_Nil or parReply.which() == RedisVariantType_String);
			if (RedisVariantType_Nil == parReply.which())
				return boost::none;
			else
				return get_string(parReply);
		}

		IncRedis::opt_string_list optional_string_list (const Reply& parReply) {
			assert(parReply.which() == RedisVariantType_Nil or parReply.which() == RedisVariantType_Array);
			if (RedisVariantType_Nil == parReply.which()) {
				return boost::none;
			}
			else {
				auto replies(get_array(parReply));
				IncRedis::opt_string_list::value_type retval;
				retval.reserve(replies.size());
				for (const auto& rep : replies) {
					retval.emplace_back(optional_string(rep));
				}
				return IncRedis::opt_string_list(std::move(retval));
			}
		}
	} //unnamed namespace

	IncRedis::IncRedis (std::string &&parAddress, uint16_t parPort) :
		m_command(std::move(parAddress), parPort)
	{
	}

	IncRedis::IncRedis (std::string&& parSocket) :
		m_command(std::move(parSocket))
	{
	}

	void IncRedis::connect() {
		m_command.connect();
	}

	void IncRedis::wait_for_connect() {
		m_command.wait_for_connect();
	}

	void IncRedis::disconnect() {
		m_command.disconnect();
	}

	void IncRedis::wait_for_disconnect() {
		m_command.wait_for_disconnect();
	}

	IncRedisBatch IncRedis::make_batch() {
		return m_command.make_batch();
	}

	auto IncRedis::scan (boost::string_view parPattern) -> scan_range {
		return scan_range(scan_iterator(&m_command, false, parPattern), scan_iterator(&m_command, true));
	}

	auto IncRedis::hscan (boost::string_view parKey, boost::string_view parPattern) -> hscan_range {
		return hscan_range(hscan_iterator(&m_command, parKey, false, parPattern), hscan_iterator(&m_command, parKey, true));
	}

	auto IncRedis::sscan (boost::string_view parKey, boost::string_view parPattern) -> sscan_range {
		return sscan_range(sscan_iterator(&m_command, parKey, false, parPattern), sscan_iterator(&m_command, parKey, true));
	}

	auto IncRedis::zscan (boost::string_view parKey, boost::string_view parPattern) -> zscan_range {
		return zscan_range(zscan_iterator(&m_command, parKey, false, parPattern), zscan_iterator(&m_command, parKey, true));
	}

	auto IncRedis::hget (boost::string_view parKey, boost::string_view parField) -> opt_string {
		return optional_string(m_command.run("HGET", parKey, parField));
	}

	RedisInt IncRedis::hincrby (boost::string_view parKey, boost::string_view parField, int parInc) {
		auto reply = m_command.run("HINCRBY", parKey, parField, int_to_ary_dec(parInc).to<boost::string_view>());
		return get_integer(reply);
	}

	auto IncRedis::srandmember (boost::string_view parKey, int parCount) -> opt_string_list {
		return optional_string_list(m_command.run("SRANDMEMBER", parKey, int_to_ary_dec(parCount).to<boost::string_view>()));
	}

	auto IncRedis::srandmember (boost::string_view parKey) -> opt_string {
		return optional_string(m_command.run("SRANDMEMBER", parKey));
	}

	auto IncRedis::smembers (boost::string_view parKey) -> opt_string_list {
		return optional_string_list(m_command.run("SMEMBERS", parKey));
	}

	auto IncRedis::zrangebyscore (boost::string_view parKey, double parMin, bool parMinIncl, double parMax, bool parMaxIncl, bool parWithScores) -> opt_string_list {
		auto batch = make_batch();
		batch.zrangebyscore(parKey, parMin, parMinIncl, parMax, parMaxIncl, parWithScores);
		assert(batch.replies().size() == 1);
		return optional_string_list(batch.replies().front());
	}

	bool IncRedis::script_flush() {
		const auto ret = redis::get<StatusString>(m_command.run("SCRIPT", "FLUSH"));
		return ret.is_ok();
	}

	bool IncRedis::flushdb() {
		const auto ret = redis::get<StatusString>(m_command.run("FLUSHDB"));
		return ret.is_ok();
	}

	RedisInt IncRedis::dbsize() {
		const auto ret = redis::get<RedisInt>(m_command.run("DBSIZE"));
		return ret;
	}

	bool IncRedis::expire (boost::string_view parKey, RedisInt parTTL) {
		const auto ret = redis::get<RedisInt>(m_command.run("EXPIRE", parKey, int_to_ary_dec(parTTL).to<boost::string_view>()));
		return (ret == 1 ? true : false);
	}

	auto IncRedis::reply_to_string_list (const Reply& parReply) -> opt_string_list {
		return optional_string_list(parReply);
	}

	auto IncRedis::get (boost::string_view parKey) -> opt_string {
		return optional_string(m_command.run("GET", parKey));
	}

	bool IncRedis::set (boost::string_view parKey, boost::string_view parField) {
		auto batch = make_batch();
		batch.set(parKey, parField, IncRedisBatch::ADD_None);
		assert(batch.replies().size() == 1);
		const auto ret = redis::get<StatusString>(batch.replies().front());
		return ret.is_ok();
	}

	RedisInt IncRedis::incr (boost::string_view parKey) {
		const auto ret = redis::get<RedisInt>(m_command.run("INCR", parKey));
		return ret;
	}
} //namespace redis
