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

#ifndef id3C772A92AB0E440DA84DAFD807BC962D
#define id3C772A92AB0E440DA84DAFD807BC962D

#include "batch.hpp"
#include "duckhandy/sequence_bt.hpp"
#include <boost/utility/string_ref.hpp>
#include <boost/lexical_cast.hpp>
#include <type_traits>

namespace redis {
	class IncRedisBatch {
	public:
		enum ZADD_Mode {
			ZADD_XX_UpdateOnly,
			ZADD_NX_AlwaysAdd,
			ZADD_None
		};

		IncRedisBatch ( void ) = delete;
		IncRedisBatch ( IncRedisBatch&& ) = default;
		IncRedisBatch ( const Batch& ) = delete;
		IncRedisBatch ( Batch&& parBatch );

		void reset ( void );
		void throw_if_failed ( void );
		const std::vector<Reply>& replies ( void ) { return m_batch.replies(); }
		Batch& batch ( void ) { return m_batch; }
		const Batch& batch ( void ) const { return m_batch; }

		//Misc
		IncRedisBatch& select ( int parIndex );
		IncRedisBatch& client_setname ( boost::string_ref parName );
		template <typename... Args>
		IncRedisBatch& del ( Args&&... parArgs );

		//Hash
		IncRedisBatch& hget ( boost::string_ref parKey, boost::string_ref parField );
		template <typename... Args>
		IncRedisBatch& hmget ( boost::string_ref parKey, Args&&... parArgs );
		template <typename... Args>
		IncRedisBatch& hmset ( boost::string_ref parKey, Args&&... parArgs );
		IncRedisBatch& hincrby ( boost::string_ref parKey, boost::string_ref parField, int parInc );

		//Set
		IncRedisBatch& srandmember ( boost::string_ref parKey, int parCount );
		IncRedisBatch& srandmember ( boost::string_ref parKey );
		template <typename... Args>
		IncRedisBatch& sadd ( boost::string_ref parKey, Args&&... parArgs );

		//Sorted set
		template <typename... Args>
		IncRedisBatch& zadd ( boost::string_ref parKey, ZADD_Mode parMode, bool parChange, Args&&... parArgs );
		IncRedisBatch& zrangebyscore ( boost::string_ref parKey, double parMin, bool parMinIncl, double parMax, bool parMaxIncl, bool parWithScores );

		//Script
		IncRedisBatch& script_flush ( void );

	private:
		Batch m_batch;
	};

	namespace implem {
		template <std::size_t... I, typename... Args>
		void run_conv_floats_to_strings ( Batch& parBatch, dhandy::bt::index_seq<I...>, Args&&... parArgs );
	} //namespace implem

	template <typename... Args>
	IncRedisBatch& IncRedisBatch::hmget (boost::string_ref parKey, Args&&... parArgs) {
		static_assert(sizeof...(Args) > 0, "No fields specified");
		m_batch.run("HMGET", parKey, std::forward<Args>(parArgs)...);
		return *this;
	}

	template <typename... Args>
	IncRedisBatch& IncRedisBatch::hmset (boost::string_ref parKey, Args&&... parArgs) {
		static_assert(sizeof...(Args) >= 1, "No parameters specified");
		static_assert(sizeof...(Args) % 2 == 0, "Uneven number of parameters received");
		m_batch.run("HMSET", parKey, std::forward<Args>(parArgs)...);
		return *this;
	}

	template <typename... Args>
	IncRedisBatch& IncRedisBatch::sadd (boost::string_ref parKey, Args&&... parArgs) {
		static_assert(sizeof...(Args) > 0, "No members specified");
		m_batch.run("SADD", parKey, std::forward<Args>(parArgs)...);
		return *this;
	}

	template <typename... Args>
	IncRedisBatch& IncRedisBatch::del (Args&&... parArgs) {
		static_assert(sizeof...(Args) > 0, "No keys specified");
		m_batch.run("DEL", std::forward<Args>(parArgs)...);
		return *this;
	}

	template <typename... Args>
	IncRedisBatch& IncRedisBatch::zadd (boost::string_ref parKey, ZADD_Mode parMode, bool parChange, Args&&... parArgs) {
		static_assert(sizeof...(Args) >= 1, "No score/value pairs specified");
		static_assert(sizeof...(Args) % 2 == 0, "Uneven number of parameters received");

		using dhandy::bt::index_range;

		if (parChange) {
			if (ZADD_None == parMode)
				implem::run_conv_floats_to_strings(m_batch, index_range<0, sizeof...(Args)>(), "ZADD", parKey, "CH", std::forward<Args>(parArgs)...);
			else if (ZADD_NX_AlwaysAdd == parMode)
				implem::run_conv_floats_to_strings(m_batch, index_range<0, sizeof...(Args)>(), "ZADD", parKey, "NX", "CH", std::forward<Args>(parArgs)...);
			else if (ZADD_XX_UpdateOnly == parMode)
				implem::run_conv_floats_to_strings(m_batch, index_range<0, sizeof...(Args)>(), "ZADD", parKey, "XX", "CH", std::forward<Args>(parArgs)...);
		}
		else {
			if (ZADD_None == parMode)
				implem::run_conv_floats_to_strings(m_batch, index_range<0, sizeof...(Args)>(), "ZADD", parKey, std::forward<Args>(parArgs)...);
			else if (ZADD_NX_AlwaysAdd == parMode)
				implem::run_conv_floats_to_strings(m_batch, index_range<0, sizeof...(Args)>(), "ZADD", parKey, "NX", std::forward<Args>(parArgs)...);
			else if (ZADD_XX_UpdateOnly == parMode)
				implem::run_conv_floats_to_strings(m_batch, index_range<0, sizeof...(Args)>(), "ZADD", parKey, "XX", std::forward<Args>(parArgs)...);
		}
		return *this;
	}

	namespace implem {
		template <std::size_t IGNORE_COUNT, std::size_t IDX, typename T, bool STRINGIZE=(IDX>=IGNORE_COUNT) && ((IDX-IGNORE_COUNT)%2)==0>
		struct stringize_or_forward_impl {
			typedef T type;
			static T&& do_it ( T&& parT ) { return std::forward<T>(parT); }
		};
		template <std::size_t IGNORE_COUNT, std::size_t IDX, typename T>
		struct stringize_or_forward_impl<IGNORE_COUNT, IDX, T, true> {
			static_assert(std::is_floating_point<T>::value, "Scores must be given as floating point values");
			typedef std::string type;
			static std::string do_it ( T parT ) { return boost::lexical_cast<std::string>(parT); }
		};

		template <std::size_t IGNORE_COUNT, std::size_t IDX, typename T>
		auto stringize_or_forward (T&& parValue) -> typename stringize_or_forward_impl<IGNORE_COUNT, IDX, T>::type {
			return stringize_or_forward_impl<IGNORE_COUNT, IDX, T>::do_it(std::forward<T>(parValue));
		}

		template <std::size_t PreArgsCount, std::size_t... I, typename... Args>
		void run_conv_floats_to_strings_impl (Batch& parBatch, dhandy::bt::index_seq<I...>, Args&&... parArgs) {
			static_assert(sizeof...(I) == sizeof...(Args), "Wrong number of indices");
			static_assert(PreArgsCount <= sizeof...(I), "Can't ignore more arguments than those that were received");
			parBatch.run(stringize_or_forward<PreArgsCount, I>(std::forward<Args>(parArgs))...);
		}

		template <std::size_t... I, typename... Args>
		void run_conv_floats_to_strings (Batch& parBatch, dhandy::bt::index_seq<I...>, Args&&... parArgs) {
			static_assert(sizeof...(Args) >= sizeof...(I), "Unexpected count, there should be at least as many argument as there are indices");
			constexpr const auto pre_args_count = sizeof...(Args) - sizeof...(I);
			run_conv_floats_to_strings_impl<pre_args_count>(parBatch, dhandy::bt::index_range<0, sizeof...(Args)>(), std::forward<Args>(parArgs)...);
		};
	} //namespace implem
} //namespace redis

#endif
