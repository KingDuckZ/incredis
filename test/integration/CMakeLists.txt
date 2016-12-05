project(integration CXX)

find_package(Boost 1.53.0 REQUIRED COMPONENTS program_options)

set(INCREDIS_TEST_HOSTNAME "127.0.0.1" CACHE STRING "Hostname for the integration test to connect to")
set(INCREDIS_TEST_PORT "6379" CACHE STRING "Port on the host")
set(INCREDIS_TEST_SOCKET "" CACHE STRING "Socket name, leave empty to use hostname:port")
set(INCREDIS_TEST_DB "0" CACHE STRING "Number of the database to run the tests in - all data in it will be destroyed")

add_executable(${PROJECT_NAME}
	main.cpp
	redis_connection_fixture.cpp
	test_insert_retrieve.cpp
	test_mass_io.cpp
)

target_include_directories(${PROJECT_NAME}
	PRIVATE ${INCREDIS_SOURCE_DIR}/lib/catch/single_include
)
target_include_directories(${PROJECT_NAME} SYSTEM
	PRIVATE ${Boost_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
	PRIVATE ${Boost_LIBRARIES}
	PRIVATE incredis
)

if ("${INCREDIS_TEST_HOSTNAME}" STREQUAL "" OR NOT "${INCREDIS_TEST_SOCKET}" STREQUAL "")
	set(hostname_param "")
else()
	set(hostname_param --hostname ${INCREDIS_TEST_HOSTNAME})
endif()
if ("${INCREDIS_TEST_PORT}" STREQUAL "" OR NOT "${INCREDIS_TEST_SOCKET}" STREQUAL "")
	set(port_param "")
else()
	set(port_param --port ${INCREDIS_TEST_PORT})
endif()
if ("${INCREDIS_TEST_SOCKET}" STREQUAL "")
	set(socket_param "")
else()
	set(socket_param --socket ${INCREDIS_TEST_SOCKET})
endif()
if ("${INCREDIS_TEST_DB}" STREQUAL "")
	set(db_param "")
else()
	set(db_param --db ${INCREDIS_TEST_DB})
endif()
add_test(
	NAME redis_integration
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	COMMAND ${PROJECT_NAME} ${hostname_param} ${port_param} ${socket_param} ${db_param}
)
