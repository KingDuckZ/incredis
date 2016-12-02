#include <cstdint>
#include <string>
#include <memory>

namespace redis {
	class IncRedis;
} //namespace redis

namespace incredis {
	namespace test {
		class RedisConnectionFixture {
		public:
			RedisConnectionFixture();
			~RedisConnectionFixture() noexcept;

			redis::IncRedis& incredis();

		private:
			bool use_socket_connection() const;

			std::unique_ptr<redis::IncRedis> m_incredis;
			std::string m_hostname;
			std::string m_socket;
			uint32_t m_db;
			uint16_t m_port;
		};
	} //namespace test
} //namespac incredis
