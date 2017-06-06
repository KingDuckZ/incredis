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

#ifndef id9348909738B047B7B6912D73CB519039
#define id9348909738B047B7B6912D73CB519039

#include "duckhandy/compatibility.h"
#include <cstddef>
#include <boost/utility/string_view.hpp>
#include <string>

namespace redis {
	namespace implem {
		template <typename T>
		const char* arg_to_bin_safe_char ( const T& parArg );

		template <typename T>
		std::size_t arg_to_bin_safe_length ( const T& parArg ) a_pure;

		template <typename T>
		struct MakeCharInfo;

		template<>
		struct MakeCharInfo<std::string> {
			MakeCharInfo ( const std::string& parData ) : m_string(parData) {}
			const char* data ( void ) const { return m_string.data(); }
			std::size_t size ( void ) const { return m_string.size(); }

		private:
			const std::string& m_string;
		};

		template<>
		struct MakeCharInfo<boost::string_view> {
			MakeCharInfo ( const boost::string_view& parData ) : m_data(parData.data()), m_size(parData.size()) {}
			const char* data ( void ) const { return m_data; }
			std::size_t size ( void ) const { return m_size; }

		private:
			const char* const m_data;
			const std::size_t m_size;
		};

		template<>
		struct MakeCharInfo<char> {
			MakeCharInfo ( char parData ) : m_data(parData) {}
			const char* data ( void ) const { return &m_data; }
			std::size_t size ( void ) const { return 1; }

		private:
			const char m_data;
		};

		template <std::size_t N>
		struct MakeCharInfo<char[N]> {
			static_assert(N > 0, "Given input should have at least one character as it's assumed to be a null-terminated string");
			MakeCharInfo ( const char (&parData)[N] ) : m_data(parData, N - 1) {}
			const char* data ( void ) const { return m_data.data(); }
			std::size_t size ( void ) const { return m_data.size(); }

		private:
			boost::string_view m_data;
		};

		template <typename T>
		inline const char* arg_to_bin_safe_char (const T& parArg) {
			return MakeCharInfo<T>(parArg).data();
		}

		template <typename T>
		inline std::size_t arg_to_bin_safe_length (const T& parArg) {
			return MakeCharInfo<T>(parArg).size();
		}
	} //namespace implem
} //namespace redis

#endif
