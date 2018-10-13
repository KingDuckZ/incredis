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

#include "incredis_batch.hpp"
#include "incredis/int_conv.hpp"
#include <sstream>
#include <utility>
#include <ciso646>

namespace redis {
	namespace {
		std::string make_boundary (double parValue, bool parExclude) {
			std::ostringstream oss;
			if (parExclude)
				oss << '(';
			oss << parValue;
			return oss.str();
		}
	} //unnamed namespace

	IncRedisBatch::IncRedisBatch (Batch&& parBatch) :
		m_batch(std::move(parBatch))
	{
	}

	void IncRedisBatch::reset() {
		m_batch.reset();
	}
	void IncRedisBatch::throw_if_failed() {
		m_batch.throw_if_failed();
	}

	IncRedisBatch& IncRedisBatch::select (int parIndex) {
		m_batch.run("SELECT", int_to_ary_dec(parIndex).to<boost::string_view>());
		return *this;
	}

	IncRedisBatch& IncRedisBatch::client_setname (boost::string_view parName) {
		m_batch.run("CLIENT", "SETNAME", parName);
		return *this;
	}

	IncRedisBatch& IncRedisBatch::hget (boost::string_view parKey, boost::string_view parField) {
		m_batch.run("HGET", parKey, parField);
		return *this;
	}

	IncRedisBatch& IncRedisBatch::hincrby (boost::string_view parKey, boost::string_view parField, int parInc) {
		m_batch.run("HINCRBY", parKey, parField, int_to_ary_dec(parInc).to<boost::string_view>());
		return *this;
	}

	IncRedisBatch& IncRedisBatch::srandmember (boost::string_view parKey, int parCount) {
		m_batch.run("SRANDMEMBER", parKey, int_to_ary_dec(parCount).to<boost::string_view>());
		return *this;
	}

	IncRedisBatch& IncRedisBatch::srandmember (boost::string_view parKey) {
		m_batch.run("SRANDMEMBER", parKey);
		return *this;
	}

	IncRedisBatch& IncRedisBatch::set (boost::string_view parKey, boost::string_view parField, ADD_Mode parMode) {
		switch(parMode) {
		case ADD_None:
			m_batch.run("SET", parKey, parField);
			break;
		case ADD_NX:
			m_batch.run("SET", parKey, parField, "NX");
			break;
		case ADD_XX:
			m_batch.run("SET", parKey, parField, "XX");
			break;
		}
		return *this;
	}

	IncRedisBatch& IncRedisBatch::zrangebyscore (boost::string_view parKey, double parMin, bool parMinIncl, double parMax, bool parMaxIncl, bool parWithScores) {
		auto lower_bound = make_boundary(parMin, not parMinIncl);
		auto upper_bound = make_boundary(parMax, not parMaxIncl);

		if (parWithScores)
			m_batch.run("ZRANGEBYSCORE", parKey, lower_bound, upper_bound, "WITHSCORES");
		else
			m_batch.run("ZRANGEBYSCORE", parKey, lower_bound, upper_bound);
		return *this;
	}

	IncRedisBatch& IncRedisBatch::script_flush() {
		m_batch.run("SCRIPT", "FLUSH");
		return *this;
	}
} //namespace redis
