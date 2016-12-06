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

#ifndef idF28625434960439DBB5B4F2A1E24CD60
#define idF28625434960439DBB5B4F2A1E24CD60

#include <forward_list>

namespace redis {
	class Reply;

	class ReplyList {
	public:
		using ReplyPtr = std::forward_list<Reply>::iterator;

		ReplyList ( void );
		~ReplyList ( void ) noexcept;

		ReplyPtr add ( void );
		std::size_t size ( void ) const;
		bool empty ( void ) const;
		std::forward_list<Reply>::iterator begin ( void );
		std::forward_list<Reply>::iterator end ( void );
		void clear ( void );

	private:
		std::forward_list<Reply> m_replies;
		std::forward_list<Reply>::iterator m_next_iterator;
		std::size_t m_size;
	};
} //namespace redis

#endif
