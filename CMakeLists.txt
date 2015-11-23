cmake_minimum_required(VERSION 3.3)
project(disk_sync)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -std=c++11 -lpthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lboost_timer")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lboost_system")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lboost_thread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -Wextra -std=c++14 -lpthread")
#project(assembler C ASM) # this enables building the assembly part of the code
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSO")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lfltk -lfltk_images")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lginac -lcln") # for ginac
project(assembler C ASM) # this enables building the assembly part of the code

add_executable(sync_images
        sync_images.cpp
        lib/coreutils/lib/sha512.c
        lib/progress_thread.cpp lib/progress_thread.h lib/config.cpp lib/config.h lib/hash_thread.cpp lib/hash_thread.h lib/immutable.cpp lib/immutable.h lib/block_device_size.cpp lib/block_device_size.h)

add_executable(test_auto_thread_count tests/test_auto_thread_count.cpp)

add_executable(test_ioctl lib/immutable.cpp tests/test_ioctl.cpp tests/test_ioctl.cpp)

add_executable(test_get_file_size tests/get_file_size.cpp)