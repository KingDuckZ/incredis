/* Copyright 2016-2018, Michele Santullo
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

#ifndef id08E89EF0951741388A48A79E769C646C
#define id08E89EF0951741388A48A79E769C646C

#include <string>
#include <stdexcept>
#include <boost/utility/string_view.hpp>
#include "duckhandy/implem/int_conv.hpp"

namespace redis {
	namespace implem {
		struct AsciiTranslator {
			static const constexpr bool BehavesLikeASCII = true;
			static const constexpr char FirstDigit = '0';
			static const constexpr char FirstLetter = 'a';
			static const constexpr char Plus = '+';
			static const constexpr char Minus = '-';

			static constexpr char to_digit (unsigned int num) {
				return (num <= 9 ? static_cast<char>(num + '0') : static_cast<char>(num + 'a'));
			}

			static constexpr int from_digit (char digit) {
				if (digit >= '0' and digit <= '9')
					return digit - '0';
				else if (digit >= 'a' and digit <= 'z')
					return digit - 'a';
				else if (digit >= 'A' and digit <= 'Z')
					return digit - 'A';
				else
					throw std::domain_error(
						std::string("Can't convert invalid character '") +
							digit + "' (" +
							std::to_string(static_cast<int>(digit)) +
							") to a number"
						);
			}
		};

		template <typename T, typename F>
		struct IntConv;

		template <typename F>
		struct IntConv<std::enable_if_t<std::is_integral_v<F>, std::string>, F> {
			static std::string conv (const F& in) {
			auto retval = dhandy::int_to_ary<F, 10, AsciiTranslator>(in);
				return std::string(retval.begin(), retval.end() - 1);
			}
		};
		template <typename T>
		struct IntConv<T, std::enable_if_t<std::is_integral_v<T>, std::string>> {
			static T conv (const std::string& in) {
				const auto size = in.size() - (in.empty() or in.back() ? 0 : 1);
				return dhandy::ary_to_int<T, char, 10, AsciiTranslator>(in.data(), in.data() + size);
			}
		};
		template <typename T>
		struct IntConv<T, std::enable_if_t<std::is_integral_v<T>, boost::string_view>> {
			static T conv (const boost::string_view& in) {
				const auto size = in.size() - (in.empty() or in.back() ? 0 : 1);
				return dhandy::ary_to_int<T, char, 10, AsciiTranslator>(in.data(), in.data() + size);
			}
		};
	} //namespace implem

	template <typename To, typename From>
	inline To int_conv (const From& from) {
		return implem::IntConv<To, From>::conv(from);
	}

	template <typename From>
	constexpr inline auto int_to_ary_hex (From f) {
		return dhandy::int_to_ary<From, 16>(f);
	}

	template <typename From>
	constexpr inline auto int_to_ary_dec (From f) {
		return dhandy::int_to_ary<From, 10>(f);
	}
} //namespace redis

#endif
