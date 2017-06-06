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

#include "script.hpp"

namespace redis {
	Script::Script() :
		m_sha1(),
		m_manager(nullptr)
	{
	}

	Script::Script (boost::string_view parSha1, ScriptManager& parManager) :
		m_sha1(parSha1),
		m_manager(&parManager)
	{
	}
} //namespace redis
