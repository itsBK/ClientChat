cmake_minimum_required(VERSION 3.10)
project(chat C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD  17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror=implicit")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror=return-type")

if("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Og -ggdb")
elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -ggdb")
endif()

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

set(SERVER_MODULES	src/broadcastagent.c
			src/clientthread.c
			src/connectionhandler.c
			src/main.c
			src/network.c
			src/user.c
			src/util.c)

include_directories(src)

add_executable(server ${SERVER_MODULES})
if(LIBRT)
	target_link_libraries(server Threads::Threads ${LIBRT})
else()
	target_link_libraries(server Threads::Threads)
endif()
