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

#ifndef idC3909E193A3C4DBEB9B646A4F5ED3518
#define idC3909E193A3C4DBEB9B646A4F5ED3518

#include <boost/range/iterator_range_core.hpp>
#include <cstddef>
#include <cassert>
#include <ciso646>

namespace redis {
	template <typename I>
	class SizedRange : public boost::iterator_range<I> {
	public:
		SizedRange ( I parBegin, I parEnd, std::size_t parSize ) :
			boost::iterator_range<I>(parBegin, parEnd),
			m_size(parSize)
		{
		}

		template <typename R>
		SizedRange ( R& parRange, std::size_t parSize ) :
			boost::iterator_range<I>(parRange),
			m_size(parSize)
		{
		}

		~SizedRange ( void ) noexcept = default;

		std::size_t size ( void ) const { return m_size; }
		bool empty ( void ) const { return static_cast<bool>(0 == m_size); }

		typename I::reference front ( void ) { assert(not empty()); return *this->begin(); }
		const typename I::reference front ( void ) const { assert(not empty()); return *this->begin(); }

	private:
		std::size_t m_size;
	};
} //namespace redis

#endif
