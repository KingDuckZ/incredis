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

#include "reply_list.hpp"
#include "reply.hpp"

namespace redis {
	ReplyList::ReplyList() :
		m_replies(),
		m_next_iterator(m_replies.before_begin()),
		m_size(0)
	{
	}

	ReplyList::~ReplyList() noexcept = default;

	auto ReplyList::add() -> ReplyPtr {
		m_next_iterator = m_replies.emplace_after(m_next_iterator);
		++m_size;
		return m_next_iterator;
	}

	std::size_t ReplyList::size() const {
		return m_size;
	}

	bool ReplyList::empty() const {
		return static_cast<bool>(0 == m_size);
	}

	std::forward_list<Reply>::iterator ReplyList::begin() {
		return m_replies.begin();
	}

	std::forward_list<Reply>::iterator ReplyList::end() {
		return m_replies.end();
	}

	void ReplyList::clear() {
		m_replies.clear();
		m_next_iterator = m_replies.begin();
		m_size = 0;
	}
} //namespace redis
