#define CATCH_CONFIG_RUNNER

#include "catch.hpp"
#include <boost/program_options.hpp>
#include <string>
#include <cstdint>
#include <algorithm>
#include <iterator>
#include <cassert>
#include <functional>

namespace po = boost::program_options;

namespace incredis {
	namespace test {
		std::string g_hostname = "127.0.0.1";
		uint16_t g_port = 6379;
		std::string g_socket = "";
		uint32_t g_db = 0;
	} //namespace test
} //namespace incredis

namespace {
	const char* const unknown_options_key = "unknown_options";

	void parse_commandline (int parArgc, const char* const* parArgv, po::variables_map& parVM) {
		po::options_description connection_options("Redis connection options");
		connection_options.add_options()
			("hostname,h", po::value<std::string>(), "Server hostname")
			("port,p", po::value<uint16_t>(), "Server port")
			("socket,s", po::value<std::string>(), "Server socket (overrides hostname and port)")
			("db,n", po::value<uint32_t>(), "Database number")
		;
		po::options_description positional_options("Catch test suite options");
		positional_options.add_options()
			(unknown_options_key, po::value<std::vector<std::string>>(), "List of options that will be passed to Catch")
		;

		po::options_description all("Available options");
		all.add(connection_options).add(positional_options);
		po::positional_options_description pd;
		pd.add(unknown_options_key, -1);

		po::store(po::command_line_parser(parArgc, parArgv).options(all).positional(pd).run(), parVM);
		po::notify(parVM);
	}

	std::vector<const char*> stringlist_to_charlist (const std::vector<std::string>& parList) {
		std::vector<const char*> retval;
		retval.reserve(parList.size());
		std::transform(parList.begin(), parList.end(), back_inserter(retval), std::bind(&std::string::c_str, std::placeholders::_1));
		assert(parList.size() == retval.size());
		return retval;
	}

	void set_global_connection_params (const po::variables_map& parVM) {
		using namespace incredis::test;

		if (parVM.count("hostname"))
			g_hostname = parVM["hostname"].as<decltype(g_hostname)>();
		if (parVM.count("port"))
			g_port = parVM["port"].as<decltype(g_port)>();
		if (parVM.count("socket"))
			g_socket = parVM["socket"].as<decltype(g_socket)>();
		if (parVM.count("db"))
			g_db = parVM["db"].as<decltype(g_db)>();
	}
} //unnamed namespace

int main (int parArgc, char* const parArgv[]) {
	po::variables_map vm;
	parse_commandline(parArgc, parArgv, vm);

	const std::vector<std::string> unparsed_params_str(
		vm.count(unknown_options_key) ?
			vm[unknown_options_key].as<std::vector<std::string>>() :
			std::vector<std::string>()
		);

	std::vector<const char*> unparsed_params =
		stringlist_to_charlist(unparsed_params_str);
	assert(parArgc);
	unparsed_params.insert(unparsed_params.begin(), parArgv[0]);

	//bleah
	set_global_connection_params(vm);

	Catch::Session session;

	const int retcode = session.applyCommandLine(unparsed_params.size(), unparsed_params.data());
	if (0 != retcode)
		return retcode;

	return session.run();
}
