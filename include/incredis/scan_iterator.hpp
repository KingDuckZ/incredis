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

#ifndef id774125B851514A26BD7C2AD1D804D732
#define id774125B851514A26BD7C2AD1D804D732

#include "reply.hpp"
#include "duckhandy/has_method.hpp"
#include "enum.h"
#include <boost/iterator/iterator_facade.hpp>
#include <type_traits>
#include <vector>
#include <cstddef>
#include <boost/utility/string_ref.hpp>

namespace redis {
	template <typename ValueFetch>
	class ScanIterator;

	class Command;

	namespace implem {
		template <typename ValueFetch>
		using ScanIteratorBaseIterator = boost::iterator_facade<ScanIterator<ValueFetch>, const typename ValueFetch::value_type, boost::forward_traversal_tag>;

		class ScanIteratorBaseClass {
		protected:
			explicit ScanIteratorBaseClass ( Command* parCommand );
			ScanIteratorBaseClass ( Command* parCommand, boost::string_ref parMatchPattern );
			~ScanIteratorBaseClass ( void ) noexcept = default;

			bool is_connected ( void ) const;
			Reply run ( const char* parCommand, long long parScanContext, std::size_t parCount );
			Reply run ( const char* parCommand, const boost::string_ref& parParameter, long long parScanContext, std::size_t parCount );

			bool is_equal ( const ScanIteratorBaseClass& parOther ) const { return m_command == parOther.m_command; }

		private:
			Command* m_command;
			boost::string_ref m_match_pattern;
		};
	} //namespace implem

	BETTER_ENUM(ScanCommands, char,
		SCAN, SSCAN, ZSCAN, HSCAN
	);

	template <typename ValueFetch>
	class ScanIterator : private implem::ScanIteratorBaseClass, public implem::ScanIteratorBaseIterator<ValueFetch>, private ValueFetch {
		friend class boost::iterator_core_access;
		typedef implem::ScanIteratorBaseIterator<ValueFetch> base_iterator;
		define_has_method(scan_target, ScanTarget);
	public:
		typedef typename base_iterator::difference_type difference_type;
		typedef typename base_iterator::value_type value_type;
		typedef typename base_iterator::pointer pointer;
		typedef typename base_iterator::reference reference;
		typedef typename base_iterator::iterator_category iterator_category;

		template <typename Dummy=ValueFetch, typename=typename std::enable_if<not HasScanTargetMethod<Dummy>::value>::type>
		ScanIterator ( Command* parCommand, bool parEnd, boost::string_ref parMatchPattern=boost::string_ref() );
		template <typename Dummy=ValueFetch, typename=typename std::enable_if<HasScanTargetMethod<Dummy>::value>::type>
		ScanIterator ( Command* parCommand, boost::string_ref parKey, bool parEnd, boost::string_ref parMatchPattern=boost::string_ref() );

	private:
		template <typename T>
		Reply forward_scan_command ( typename std::enable_if<HasScanTargetMethod<T>::value, long long>::type parContext );
		template <typename T>
		Reply forward_scan_command ( typename std::enable_if<not HasScanTargetMethod<T>::value, long long>::type parContext );
		bool is_end ( void ) const;

		void increment ( void );
		bool equal ( const ScanIterator& parOther ) const;
		const value_type& dereference ( void ) const;

		std::vector<value_type> m_reply;
		long long m_scan_context;
		std::size_t m_curr_index;
	};

	template <typename T>
	struct ScanSingleValues {
		typedef T value_type;

		static constexpr const char* command ( void ) { return "SCAN"; }
		static constexpr const std::size_t step = 1;
		static constexpr const std::size_t work_count = 10;

		static const T& make_value ( const Reply* parItem );
	};

	template <typename T>
	struct ScanSingleValuesInKey {
		typedef T value_type;

		explicit ScanSingleValuesInKey ( boost::string_ref parScanTarget ) : m_scan_target(parScanTarget) {}

		static constexpr const char* command ( void ) { return "SSCAN"; }
		static constexpr const std::size_t step = 1;
		static constexpr const std::size_t work_count = 10;

		static const T& make_value ( const Reply* parItem );
		boost::string_ref scan_target ( void ) const { return m_scan_target; }

	private:
		boost::string_ref m_scan_target;
	};

	template <typename P, char Command, typename A=decltype(P().first), typename B=decltype(P().second)>
	struct ScanPairs {
		static_assert(Command == ScanCommands::HSCAN or Command == ScanCommands::ZSCAN, "Invalid scan command chosen");
		typedef P value_type;

		explicit ScanPairs ( boost::string_ref parScanTarget ) : m_scan_target(parScanTarget) {}

		static constexpr const char* command ( void ) { return ScanCommands::_from_integral(Command)._to_string(); }
		static constexpr const std::size_t step = 2;
		static constexpr const std::size_t work_count = 10;

		static value_type make_value ( const Reply* parItem );
		boost::string_ref scan_target ( void ) const { return m_scan_target; }

	private:
		boost::string_ref m_scan_target;
	};
} //namespace redis

#include "scan_iterator.inl"

#endif
