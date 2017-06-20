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
#include "script_manager.hpp"
#include "async_connection.hpp"
#include "thread_context.hpp"
#include <hiredis/hiredis.h>
#include <ciso646>
#include <cassert>
#include <algorithm>
#include <stdexcept>

//See docs directory for info about hiredis/libev with multithreading

namespace redis {
	namespace {
	} //unnamed namespace

	struct Command::LocalData {
		explicit LocalData (Command* parCommand, std::string&& parAddress, uint16_t parPort) :
			async_connection(std::move(parAddress), parPort),
			lua_scripts(parCommand)
		{
		}

		AsyncConnection async_connection;
		ScriptManager lua_scripts;
		ThreadContext thread_context;
	};

	Command::Command (std::string&& parAddress, uint16_t parPort) :
		m_local_data(new LocalData(this, std::move(parAddress), parPort))
	{
	}

	Command::Command (Command&& parOther) :
		m_local_data(std::move(parOther.m_local_data))
	{
		m_local_data->lua_scripts.update_command_ptr(this);
	}

	Command::Command (std::string&& parSocket) :
		Command(std::move(parSocket), 0)
	{
	}

	Command::~Command() noexcept = default;

	void Command::connect() {
		m_local_data->async_connection.connect();
	}

	void Command::wait_for_connect() {
		m_local_data->async_connection.wait_for_connect();
	}

	void Command::disconnect() {
		m_local_data->async_connection.disconnect();
	}

	void Command::wait_for_disconnect() {
		m_local_data->async_connection.wait_for_disconnect();
	}

	bool Command::is_connected() const {
		return m_local_data->async_connection.is_connected();
	}

	boost::string_view Command::connection_error() const {
		return m_local_data->async_connection.connection_error();
	}

	Batch Command::make_batch() {
		assert(is_connected());
		return Batch(&m_local_data->async_connection, m_local_data->thread_context);
	}

	Script Command::make_script (const std::string &parScript) {
		auto sha1 = m_local_data->lua_scripts.submit_lua_script(parScript);
		return Script(sha1, m_local_data->lua_scripts);
	}
} //namespace redis
