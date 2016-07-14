cmake_minimum_required(VERSION 3.3 FATAL_ERROR)
project(incredis CXX)
list (APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules)

include(shared_git_project)

find_package(hiredis 0.11.0 REQUIRED)
find_package(CryptoPP 5.6)
find_package(libev 4.0 REQUIRED)
find_package(Boost 1.53.0 REQUIRED)

add_shared_git_project("lib/duckhandy")

add_library(${PROJECT_NAME} SHARED
	src/command.cpp
	src/scan_iterator.cpp
	src/reply.cpp
	src/batch.cpp
	src/script.cpp
	src/script_manager.cpp
	src/async_connection.cpp
	src/incredis.cpp
	src/incredis_batch.cpp
)

target_include_directories(${PROJECT_NAME} SYSTEM
	PUBLIC ${Boost_INCLUDE_DIRS}
	PRIVATE ${HIREDIS_INCLUDE_DIRS}
	PUBLIC ${CMAKE_SOURCE_DIR}/lib/better-enums
	PRIVATE ${LIBEV_INCLUDE_DIRS}
	PRIVATE ${Boost_INCLUDE_DIRS}
)
target_include_directories(${PROJECT_NAME}
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
	PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/incredis
)

target_link_libraries(${PROJECT_NAME}
	PRIVATE ${HIREDIS_LIBRARIES}
	PRIVATE ${LIBEV_LIBRARIES}
	PRIVATE ${Boost_LIBRARIES}
	PUBLIC duckhandy
)

configure_file(
	src/incredisConfig.h.in
	${CMAKE_CURRENT_BINARY_DIR}/incredisConfig.h
)

target_compile_definitions(${PROJECT_NAME}
	PRIVATE EV_COMPAT3=0
)

if (CryptoPP_FOUND)
	target_link_libraries(${PROJECT_NAME} PRIVATE ${CryptoPP_LIBRARIES})
	target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE ${CryptoPP_INCLUDE_DIRS})
	set (has_cryptopp_lib ON)
else()
	set (has_cryptopp_lib OFF)
endif()

install(TARGETS ${PROJECT_NAME}
	LIBRARY DESTINATION lib
	RUNTIME DESTINATION bin
	ARCHIVE DESTINATION lib/static
)

target_compile_features(${PROJECT_NAME}
	PUBLIC cxx_nullptr
	PUBLIC cxx_range_for
	PUBLIC cxx_lambdas
	PUBLIC cxx_decltype_auto
	PUBLIC cxx_defaulted_functions
	PUBLIC cxx_deleted_functions
	PUBLIC cxx_auto_type
	PUBLIC cxx_decltype_incomplete_return_types
	PUBLIC cxx_defaulted_move_initializers
	PUBLIC cxx_noexcept
	PUBLIC cxx_rvalue_references
)
