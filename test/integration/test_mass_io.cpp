#include "redis_connection_fixture.hpp"
#include "catch.hpp"
#include "incredis/incredis.hpp"
#include "duckhandy/lengthof.h"
#include <unordered_map>
#include <string>
#include <algorithm>
#include <random>
#include <chrono>
#include <iostream>

using incredis::test::RedisConnectionFixture;

namespace {
	const char g_charset[] = {
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
		'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
		'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
		'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd',
		'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
		'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x',
		'y', 'z'
	};

	template <typename F>
	std::string make_random_string (std::size_t parLength, F parGetRandomChar) {
		std::string str_ret(parLength, 0);
		std::generate_n(str_ret.begin(), parLength, parGetRandomChar);
		return str_ret;
	}

	std::unordered_map<std::string, std::string> generate_random_data (std::size_t parItemCount, std::size_t parKeyLength, std::size_t parValueLength) {
		std::default_random_engine rng(std::random_device{}());
		std::uniform_int_distribution<> dist(0, lengthof(g_charset) - 1);

		std::unordered_map<std::string, std::string> random_strings;
		auto get_rand_char = [&dist, &rng]() { return g_charset[dist(rng)]; };
		for (std::size_t z = 0; z < parItemCount; ++z) {
			std::string rand_key;
			do {
				rand_key = make_random_string(parKeyLength, get_rand_char);
			} while (random_strings.count(rand_key) > 0);
			random_strings[std::move(rand_key)] = make_random_string(parValueLength, get_rand_char);
		}

		return random_strings;
	}
} //unnamed namespace

TEST_CASE_METHOD(RedisConnectionFixture, "Insert a large amount of elements and take the timing", "[set]") {
	using redis::IncRedisBatch;

	const std::size_t items_count = 1500000;
	const std::size_t rand_key_length = 10;
	const std::size_t rand_val_length = 9;

	incredis().flushdb();

	std::cout << "Generating " << items_count << " random elements..." << std::endl;
	const auto random_strings = generate_random_data(items_count, rand_key_length, rand_val_length);
	auto batch = incredis().make_batch();

	std::cout << "Inserting into db..." << std::endl;
	std::chrono::system_clock::time_point start = std::chrono::high_resolution_clock::now();
	for (auto& str : random_strings) {
		batch.set(str.first, str.second, IncRedisBatch::ADD_None);
	}
	std::chrono::system_clock::time_point send_end = std::chrono::high_resolution_clock::now();
	batch.replies();
	std::chrono::system_clock::time_point batch_end = std::chrono::high_resolution_clock::now();

	REQUIRE_NOTHROW(batch.throw_if_failed());

	std::cout << "Inserted " << random_strings.size() << " elements, loop completed in " <<
		std::chrono::duration_cast<std::chrono::milliseconds>(send_end - start).count() << "ms" <<
		", batch finished in " <<
		std::chrono::duration_cast<std::chrono::milliseconds>(batch_end - start).count() << "ms" <<
		", average time per element: " <<
		std::chrono::duration_cast<std::chrono::microseconds>(batch_end - start).count() /
			static_cast<double>(random_strings.size()) <<
		"us, dbsize: " << incredis().dbsize() <<
		"\n"
	;
}
