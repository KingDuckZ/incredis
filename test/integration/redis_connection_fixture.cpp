#include "redis_connection_fixture.hpp"
#include "catch.hpp"
#include "incredis/incredis.hpp"
#include <ciso646>

namespace incredis {
	namespace test {
		extern std::string g_hostname;
		extern uint16_t g_port;
		extern std::string g_socket;
		extern uint32_t g_db;

		RedisConnectionFixture::RedisConnectionFixture() :
			m_hostname(g_hostname),
			m_socket(g_socket),
			m_db(g_db),
			m_port(g_port)
		{
			if (use_socket_connection())
				m_incredis.reset(new redis::IncRedis(std::string(m_socket)));
			else
				m_incredis.reset(new redis::IncRedis(std::string(m_hostname), m_port));
			m_incredis->connect();
			m_incredis->wait_for_connect();
			REQUIRE_FALSE(not m_incredis->is_connected());

			auto batch = m_incredis->make_batch();
			batch.select(m_db);
			batch.client_setname("IncredisIntegrationTest");
			batch.script_flush();
			REQUIRE_NOTHROW(batch.throw_if_failed());
		}

		RedisConnectionFixture::~RedisConnectionFixture() noexcept = default;

		bool RedisConnectionFixture::use_socket_connection() const {
			return not m_socket.empty();
		}

		redis::IncRedis& RedisConnectionFixture::incredis() {
			return *m_incredis;
		}
	} //namespace test
} //namespace incredis
