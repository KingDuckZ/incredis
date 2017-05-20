cmake_minimum_required(VERSION 3.3 FATAL_ERROR)
project(incredis VERSION 0.1.0 LANGUAGES CXX)
list (APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules)

include(shared_git_project)
include(CTest)

option(INCREDIS_FORCE_DISABLE_TESTS "Ignore unit tests even if BUILD_TESTING is set to ON - useful if you want to disable incredis tests from your top-level cmake project" OFF)
set(CMAKE_INSTALL_INCLUDEDIR "" CACHE PATH "Specify the output directory for header files (default is include)")
set(CMAKE_INSTALL_LIBDIR "" CACHE PATH "Specify the output directory for libraries (default is lib)")
set(CMAKE_INSTALL_PKGCONFIGDIR "" CACHE PATH "Specify the output directory for pkgconfig files (default is lib/pkgconfig)")
set(PROJECT_URL "https://github.com/KingDuckZ/incredis")
set(PROJECT_DESCRIPTION "A lightweight C++ wrapper around hiredis")

if(NOT CMAKE_INSTALL_INCLUDEDIR)
	set(CMAKE_INSTALL_INCLUDEDIR "${CMAKE_INSTALL_PREFIX}/include")
endif(NOT CMAKE_INSTALL_INCLUDEDIR)
if(NOT CMAKE_INSTALL_LIBDIR)
	set(CMAKE_INSTALL_LIBDIR "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}")
endif(NOT CMAKE_INSTALL_LIBDIR)
if(NOT CMAKE_INSTALL_PKGCONFIGDIR)
	set(CMAKE_INSTALL_PKGCONFIGDIR "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
endif(NOT CMAKE_INSTALL_PKGCONFIGDIR)

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
	src/reply_list.cpp
)

target_include_directories(${PROJECT_NAME} SYSTEM
	PUBLIC ${Boost_INCLUDE_DIRS}
	PRIVATE ${HIREDIS_INCLUDE_DIRS}
	PUBLIC lib/better-enums
	PRIVATE ${LIBEV_INCLUDE_DIRS}
)
target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>
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
configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/pkgconfig/incredis.pc.in"
	"${CMAKE_CURRENT_BINARY_DIR}/incredis.pc"
	@ONLY
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
set(INCREDIS_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}Config
	LIBRARY DESTINATION lib
	RUNTIME DESTINATION bin
	ARCHIVE DESTINATION lib/static
)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT ${PROJECT_NAME}Config DESTINATION share/${PROJECT_NAME}/cmake)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/incredis.pc" DESTINATION ${CMAKE_INSTALL_PKGCONFIGDIR})

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

if (BUILD_TESTING AND NOT INCREDIS_FORCE_DISABLE_TESTS)
	add_subdirectory(test/integration)
endif()
