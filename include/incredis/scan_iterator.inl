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

#include "command.hpp"
#include <cassert>
#include <ciso646>

namespace redis {
	namespace implem {
	} //namespace implem

	template <typename ValueFetch>
	template <typename Dummy, typename>
	ScanIterator<ValueFetch>::ScanIterator (Command* parCommand, bool parEnd, boost::string_ref parMatchPattern) :
		implem::ScanIteratorBaseClass(parCommand, parMatchPattern),
		implem::ScanIteratorBaseIterator<ValueFetch>(),
		ValueFetch(),
		m_reply(),
		m_scan_context(0),
		m_curr_index(0)
	{
		if (not parEnd) {
			m_curr_index = 1; //Some arbitrary value so is_end()==false
			assert(not is_end());
			this->increment();
		}
		else {
			assert(is_end());
		}
	}

	template <typename ValueFetch>
	template <typename Dummy, typename>
	ScanIterator<ValueFetch>::ScanIterator (Command* parCommand, boost::string_ref parKey, bool parEnd, boost::string_ref parMatchPattern) :
		implem::ScanIteratorBaseClass(parCommand, parMatchPattern),
		implem::ScanIteratorBaseIterator<ValueFetch>(),
		ValueFetch(parKey),
		m_reply(),
		m_scan_context(0),
		m_curr_index(0)
	{
		if (not parEnd) {
			m_curr_index = 1; //Some arbitrary value so is_end()==false
			assert(not is_end());
			this->increment();
		}
		else {
			assert(is_end());
		}
	}

	template <typename ValueFetch>
	bool ScanIterator<ValueFetch>::is_end() const {
		return not m_curr_index and m_reply.empty() and not m_scan_context;
	}

	template <typename ValueFetch>
	void ScanIterator<ValueFetch>::increment() {
		assert(not is_end());
		static_assert(ValueFetch::step > 0, "Can't have an increase step of 0");

		if (m_curr_index + 1 < m_reply.size()) {
			++m_curr_index;
		}
		else if (m_curr_index + 1 == m_reply.size() and not m_scan_context)	{
			m_reply.clear();
			m_curr_index = 0;
		}
		else {
			std::vector<Reply> array_reply;
			RedisInt new_context = m_scan_context;

			do {
				auto whole_reply = this->forward_scan_command<ValueFetch>(new_context);

				array_reply = get_array(whole_reply);
				assert(2 == array_reply.size());
				assert(array_reply.size() % ValueFetch::step == 0);
				new_context = get_integer_autoconv_if_str(array_reply[0]);
			} while (new_context and get_array(array_reply[1]).empty());

			const auto variant_array = get_array(array_reply[1]);
			assert(variant_array.size() % ValueFetch::step == 0);
			const std::size_t expected_reply_count = variant_array.size() / ValueFetch::step;
			m_reply.clear();
			m_reply.reserve(expected_reply_count);
			for (std::size_t z = 0; z < variant_array.size(); z += ValueFetch::step) {
				m_reply.push_back(ValueFetch::make_value(variant_array.data() + z));
			}
			assert(expected_reply_count == m_reply.size());
			m_scan_context = new_context;
			m_curr_index = 0;
		}
	}

	template <typename ValueFetch>
	bool ScanIterator<ValueFetch>::equal (const ScanIterator& parOther) const {
		return
			(&parOther == this) or
			(is_end() and parOther.is_end()) or
			(
				not (is_end() or parOther.is_end()) and
				implem::ScanIteratorBaseClass::is_equal(parOther) and
				(m_scan_context == parOther.m_scan_context) and
				(m_curr_index == parOther.m_curr_index) and
				(m_reply.size() == parOther.m_reply.size())
			);
	}

	template <typename ValueFetch>
	auto ScanIterator<ValueFetch>::dereference() const -> const value_type& {
		assert(not m_reply.empty());
		assert(m_curr_index < m_reply.size());

		return m_reply[m_curr_index];
	}

	template <typename ValueFetch>
	template <typename T>
	Reply ScanIterator<ValueFetch>::forward_scan_command (typename std::enable_if<HasScanTargetMethod<T>::value, RedisInt>::type parContext) {
		return implem::ScanIteratorBaseClass::run(T::command(), T::scan_target(), parContext, T::work_count);
	}

	template <typename ValueFetch>
	template <typename T>
	Reply ScanIterator<ValueFetch>::forward_scan_command (typename std::enable_if<not HasScanTargetMethod<T>::value, RedisInt>::type parContext) {
		return implem::ScanIteratorBaseClass::run(T::command(), parContext, T::work_count);
	}

	template <typename T>
	auto ScanSingleValues<T>::make_value (const Reply* parItem) -> const value_type& {
		assert(parItem);
		return get<T>(*parItem);
	}

	template <typename T>
	auto ScanSingleValuesInKey<T>::make_value (const Reply* parItem) -> const value_type& {
		assert(parItem);
		return get<T>(*parItem);
	}

	template <typename P, char Command, typename A, typename B>
	auto ScanPairs<P, Command, A, B>::make_value (const Reply* parItem) -> value_type {
		assert(parItem);
		return value_type(get<A>(parItem[0]), get<B>(parItem[1]));
	}
} //namespace redis
