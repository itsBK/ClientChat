cmake_minimum_required(VERSION 3.10)
project(chat CXX)

set(CMAKE_CXX_STANDARD  17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=return-type")

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og -ggdb")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb")
endif()

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

set(SERVER_MODULES
	src/clientthread.cpp
	src/server.cpp
	src/main.cpp
	src/network.cpp
	src/user.cpp
	src/util.cpp
	src/cmdHelper.hpp)

include_directories(src)

add_executable(server ${SERVER_MODULES})
if(LIBRT)
	target_link_libraries(server Threads::Threads ${LIBRT})
else()
	target_link_libraries(server Threads::Threads)
endif()
