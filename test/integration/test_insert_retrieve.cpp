#include "redis_connection_fixture.hpp"
#include "catch.hpp"
#include "incredis/incredis.hpp"
#include <unordered_map>
#include <string>

using incredis::test::RedisConnectionFixture;

TEST_CASE_METHOD(RedisConnectionFixture, "Batch insert elements in the db and expect to be able to read the same values back", "[set][get]") {
	using redis::IncRedisBatch;

	REQUIRE_FALSE(not incredis().flushdb());
	REQUIRE(incredis().dbsize() == 0);

	auto batch = incredis().make_batch();
	const std::unordered_map<std::string, std::string> sample_values {
		{"個人と対話", "プロセスやツール"},
		{"動くソフトウェア", "包括的なドキュメント"},
		{"顧客との協調", "契約交渉"},
		{"変化への対応", "計画に従うこと"},
		{"erase", "erases elements"},
		{"swap", "swaps the contents"},
		{"extract", "extracts nodes from the container"},
		{"merge", "splices nodes from another container"}
	};

	for (auto& value : sample_values) {
		batch.set(value.first, value.second, IncRedisBatch::ADD_NX);
	}
	REQUIRE_NOTHROW(batch.throw_if_failed());

	REQUIRE(incredis().dbsize() == static_cast<redis::RedisInt>(sample_values.size()));

	{
		redis::RedisInt scanned_items = 0;
		for (auto& key : incredis().scan()) {
			REQUIRE(sample_values.count(key) == 1);
			redis::IncRedis::opt_string value = incredis().get(key);
			REQUIRE_FALSE(not value);
			REQUIRE(*value == sample_values.at(key));
			++scanned_items;
		}
		REQUIRE(static_cast<redis::RedisInt>(sample_values.size()) == scanned_items);
	}
}

TEST_CASE_METHOD(RedisConnectionFixture, "Insert elements in the db and expect to be able to read the same values back", "[set][get]") {
	using redis::IncRedisBatch;

	REQUIRE_FALSE(not incredis().flushdb());
	REQUIRE(incredis().dbsize() == 0);

	incredis().set("個人と対話", "プロセスやツール");
	incredis().set("動くソフトウェア", "包括的なドキュメント");
	incredis().set("顧客との協調", "契約交渉");
	incredis().set("変化への対応", "計画に従うこと");
	incredis().set("erase", "erases elements");
	incredis().set("swap", "swaps the contents");
	incredis().set("extract", "extracts nodes from the container");
	incredis().set("merge", "splices nodes from another container");

	REQUIRE(incredis().dbsize() == 8);

	REQUIRE_FALSE(not incredis().get("個人と対話"));
	REQUIRE_FALSE(not incredis().get("動くソフトウェア"));
	REQUIRE_FALSE(not incredis().get("顧客との協調"));
	REQUIRE_FALSE(not incredis().get("変化への対応"));
	REQUIRE_FALSE(not incredis().get("erase"));
	REQUIRE_FALSE(not incredis().get("swap"));
	REQUIRE_FALSE(not incredis().get("extract"));
	REQUIRE_FALSE(not incredis().get("merge"));

	REQUIRE(*incredis().get("個人と対話") == "プロセスやツール");
	REQUIRE(*incredis().get("動くソフトウェア") == "包括的なドキュメント");
	REQUIRE(*incredis().get("顧客との協調") == "契約交渉");
	REQUIRE(*incredis().get("変化への対応") == "計画に従うこと");
	REQUIRE(*incredis().get("erase") == "erases elements");
	REQUIRE(*incredis().get("swap") == "swaps the contents");
	REQUIRE(*incredis().get("extract") == "extracts nodes from the container");
	REQUIRE(*incredis().get("merge") == "splices nodes from another container");
}
