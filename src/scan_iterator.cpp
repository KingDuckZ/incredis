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

#include "scan_iterator.hpp"
#include "duckhandy/lexical_cast.hpp"
#include "command.hpp"
#include <cassert>
#include <ciso646>
#include <string>

namespace redis {
	namespace implem {
		ScanIteratorBaseClass::ScanIteratorBaseClass (Command* parCommand) :
			ScanIteratorBaseClass(parCommand, boost::string_ref())
		{
		}

		ScanIteratorBaseClass::ScanIteratorBaseClass (Command* parCommand, boost::string_ref parMatchPattern) :
			m_command(parCommand),
			m_match_pattern(parMatchPattern)
		{
			assert(m_command);
			assert(m_command->is_connected());
		}

		bool ScanIteratorBaseClass::is_connected() const {
			return m_command and m_command->is_connected();
		}

		Reply ScanIteratorBaseClass::run (const char* parCommand, long long parScanContext, std::size_t parCount) {
			const auto scan_context = dhandy::lexical_cast<std::string>(parScanContext);
			const auto count_hint = dhandy::lexical_cast<std::string>(parCount);
			if (m_match_pattern.empty())
				return m_command->run(parCommand, scan_context, "COUNT", count_hint);
			else
				return m_command->run(parCommand, scan_context, "MATCH", m_match_pattern, "COUNT", count_hint);
		}

		Reply ScanIteratorBaseClass::run (const char* parCommand, const boost::string_ref& parParameter, long long parScanContext, std::size_t parCount) {
			const auto scan_context = dhandy::lexical_cast<std::string>(parScanContext);
			const auto count_hint = dhandy::lexical_cast<std::string>(parCount);
			if (m_match_pattern.empty())
				return m_command->run(parCommand, parParameter, scan_context, "COUNT", count_hint);
			else
				return m_command->run(parCommand, parParameter, scan_context, "MATCH", m_match_pattern, "COUNT", count_hint);
		}
	} //namespace implem
} //namespace redis
