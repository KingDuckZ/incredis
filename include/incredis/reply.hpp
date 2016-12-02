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

#ifndef id93FA96E3071745D9A1E45D4D29B9F7D0
#define id93FA96E3071745D9A1E45D4D29B9F7D0

#include <boost/variant/variant.hpp>
#include <string>
#include <vector>

namespace redis {
	typedef long long RedisInt;
	struct Reply;

	class ErrorString {
	public:
		ErrorString ( const char* parCStr, std::size_t parLen ) :
			m_msg(parCStr, parLen)
		{ }
		const std::string& message ( void ) const noexcept { return m_msg; }

	private:
		std::string m_msg;
	};
	class StatusString {
	public:
		StatusString ( const char* parCStr, std::size_t parLen ) :
			m_msg(parCStr, parLen)
		{ }
		const std::string& message ( void ) const noexcept { return m_msg; }
		bool is_ok ( void ) const { return "OK" == m_msg; }

	private:
		std::string m_msg;
	};

	namespace implem {
		using RedisVariantType = boost::variant<
			RedisInt,
			std::string,
			std::vector<Reply>,
			ErrorString,
			StatusString,
			std::nullptr_t
		>;
	} //namespace implem
	enum RedisVariantTypes {
		RedisVariantType_Integer = 0,
		RedisVariantType_String,
		RedisVariantType_Array,
		RedisVariantType_Error,
		RedisVariantType_Status,
		RedisVariantType_Nil
	};

	struct Reply : implem::RedisVariantType {
		using base_class = implem::RedisVariantType;

		Reply ( void ) = default;
		Reply ( RedisInt parVal ) : base_class(parVal) {}
		Reply ( std::string&& parVal ) : base_class(std::move(parVal)) {}
		Reply ( std::vector<Reply>&& parVal ) : base_class(std::move(parVal)) {}
		Reply ( ErrorString&& parVal ) : base_class(std::move(parVal)) {}
		Reply ( StatusString&& parVal ) : base_class(std::move(parVal)) {}
		Reply ( std::nullptr_t parVal ) : base_class(parVal) {}
		~Reply ( void ) noexcept = default;

		bool is_integer ( void ) const;
		bool is_string ( void ) const;
		bool is_array ( void ) const;
		bool is_error ( void ) const;
		bool is_status ( void ) const;
		bool is_nil ( void ) const;
	};

	const RedisInt& get_integer ( const Reply& parReply );
	RedisInt get_integer_autoconv_if_str ( const Reply& parReply );
	const std::string& get_string ( const Reply& parReply );
	const std::vector<Reply>& get_array ( const Reply& parReply );
	const ErrorString& get_error_string ( const Reply& parReply );

	template <typename T>
	const T& get ( const Reply& parReply );
} //namespace redis

#endif
