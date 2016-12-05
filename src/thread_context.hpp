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

#ifndef idCF662C64AAB440879A3BA23C74AFF9BF
#define idCF662C64AAB440879A3BA23C74AFF9BF

#include <atomic>

namespace redis {
	struct ThreadContext {
		ThreadContext() :
			pending_futures(0)
		{
		}

		std::atomic_size_t pending_futures;
	};
} //namespace redis

#endif
