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

#include "script_manager.hpp"
#include "duckhandy/lexical_cast.hpp"
#include "command.hpp"
#include <cassert>
#if defined(MAKE_SHA1_WITH_CRYPTOPP)
#	include <crypto++/sha.h>
#endif

namespace redis {
	namespace {
#if defined(MAKE_SHA1_WITH_CRYPTOPP)
		struct LuaScriptHash {
			union {
				struct {
					uint64_t part_a, part_b;
					uint32_t part_c;
				};
				uint8_t raw_bytes[20];
			};
		};
#endif
	} //unnamed namespace

	ScriptManager::ScriptManager (Command* parCommand) :
		m_command(parCommand)
	{
		assert(m_command);
	}

#if defined(MAKE_SHA1_WITH_CRYPTOPP)
	boost::string_view ScriptManager::add_lua_script_ifn (const std::string& parScript) {
		assert(m_command->is_connected());

		if (parScript.empty())
			return boost::string_view();

		using dhandy::lexical_cast;

		static_assert(20 == CryptoPP::SHA1::DIGESTSIZE, "Unexpected SHA1 digest size");
		static_assert(sizeof(LuaScriptHash) >= CryptoPP::SHA1::DIGESTSIZE, "Wrong SHA1 struct size");
		static_assert(Sha1Array().size() == CryptoPP::SHA1::DIGESTSIZE * 2, "Wrong array size");

		LuaScriptHash digest;
		CryptoPP::SHA1().CalculateDigest(digest.raw_bytes, reinterpret_cast<const uint8_t*>(parScript.data()), parScript.size());
		//TODO: change when lexical_cast will support arrays
		auto sha1_str_parta = lexical_cast<std::string, dhandy::tags::hexl>(__builtin_bswap64(digest.part_a));
		auto sha1_str_partb = lexical_cast<std::string, dhandy::tags::hexl>(__builtin_bswap64(digest.part_b));
		auto sha1_str_partc = lexical_cast<std::string, dhandy::tags::hexl>(__builtin_bswap32(digest.part_c));
		const std::string sha1_str =
			std::string(sizeof(digest.part_a) * 2 - sha1_str_parta.size(), '0') + sha1_str_parta +
			std::string(sizeof(digest.part_b) * 2 - sha1_str_partb.size(), '0') + sha1_str_partb +
			std::string(sizeof(digest.part_c) * 2 - sha1_str_partc.size(), '0') + sha1_str_partc
		;
		Sha1Array sha1_array;
		assert(sha1_str.size() == sha1_array.size());
		std::copy(sha1_str.begin(), sha1_str.end(), sha1_array.begin());

		auto it_found = m_known_hashes.find(sha1_array);
		const bool was_present = (m_known_hashes.end() != it_found);
		if (was_present) {
			return boost::string_view(it_found->data(), it_found->size());
		}

		auto reply = m_command->run("SCRIPT", "LOAD", parScript);
		assert(not was_present);

		assert(get_string(reply) == sha1_str);
		const auto it_inserted = m_known_hashes.insert(it_found, sha1_array);
		(void)reply;

		return boost::string_view(it_inserted->data(), it_inserted->size());
	}
#else
	boost::string_view ScriptManager::add_lua_script_ifn (const std::string& parScript) {
		assert(m_command->is_connected());

		auto it_found = m_known_scripts.find(parScript);
		const bool was_present = (m_known_scripts.end() != it_found);
		if (was_present) {
			return boost::string_view(it_found->second.data(), it_found->second.size());
		}

		auto reply = m_command->run("SCRIPT", "LOAD", parScript);
		assert(not was_present);

		const auto sha1_str = get_string(reply);
		Sha1Array sha1_array;
		std::copy(sha1_str.begin(), sha1_str.end(), sha1_array.begin());
		auto it_inserted = m_known_scripts.insert(it_found, std::make_pair(parScript, sha1_array));

		return boost::string_view(it_inserted->second.data(), it_inserted->second.size());
	}
#endif

	void ScriptManager::update_command_ptr (Command* parNewPtr) {
		assert(parNewPtr);
		m_command = parNewPtr;
	}
} //namespace redis
