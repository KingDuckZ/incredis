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

#ifndef id5B30CDA57F894CD6888093B64F9433DA
#define id5B30CDA57F894CD6888093B64F9433DA

#include "batch.hpp"
#include "duckhandy/lexical_cast.hpp"
#include "duckhandy/sequence_bt.hpp"
#include <boost/utility/string_ref.hpp>
#include <tuple>
#include <cassert>
#include <ciso646>

namespace redis {
	class ScriptManager;

	class Script {
	public:
		Script ( void );
		Script ( Script&& ) = default;
		Script ( boost::string_ref parSha1, ScriptManager& parManager );
		~Script ( void ) noexcept = default;

		template <typename... Keys, typename... Values>
		void run ( Batch& parBatch, const std::tuple<Keys...>& parKeys, const std::tuple<Values...>& parValues );

		Script& operator= ( Script&& ) = default;

	private:
		template <typename... Keys, typename... Values, std::size_t... KeyIndices, std::size_t... ValueIndices>
		void run_with_indices ( Batch& parBatch, const std::tuple<Keys...>& parKeys, const std::tuple<Values...>& parValues, dhandy::bt::index_seq<KeyIndices...>, dhandy::bt::index_seq<ValueIndices...> );

		boost::string_ref m_sha1;
		ScriptManager* m_manager;
	};

	template <typename... Keys, typename... Values>
	void Script::run (Batch& parBatch, const std::tuple<Keys...>& parKeys, const std::tuple<Values...>& parValues) {
		this->run_with_indices(
			parBatch,
			parKeys,
			parValues,
			::dhandy::bt::index_range<0, sizeof...(Keys)>(),
			::dhandy::bt::index_range<0, sizeof...(Values)>()
		);
	}

	template <typename... Keys, typename... Values, std::size_t... KeyIndices, std::size_t... ValueIndices>
	void Script::run_with_indices (Batch& parBatch, const std::tuple<Keys...>& parKeys, const std::tuple<Values...>& parValues, dhandy::bt::index_seq<KeyIndices...>, dhandy::bt::index_seq<ValueIndices...>) {
		static_assert(sizeof...(Keys) == sizeof...(KeyIndices), "Wrong index count");
		static_assert(sizeof...(Values) == sizeof...(ValueIndices), "Wrong value count");
		static_assert(sizeof...(Keys) == std::tuple_size<std::tuple<Keys...>>::value, "Wrong key count");
		static_assert(sizeof...(Values) == std::tuple_size<std::tuple<Values...>>::value, "Wrong value count");

		assert(not m_sha1.empty());
		assert(m_manager);

		parBatch.run(
			"EVALSHA",
			m_sha1,
			dhandy::lexical_cast<std::string>(sizeof...(Keys)),
			std::get<KeyIndices>(parKeys)...,
			std::get<ValueIndices>(parValues)...
		);
	}
} //namespace redis

#endif
