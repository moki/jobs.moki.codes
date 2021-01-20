cmake_minimum_required(VERSION 3.10)

project(daemon)

SET (CMAKE_C_COMPILER             "/usr/bin/clang")
SET (CMAKE_C_FLAGS                "-Wall -std=c99")
SET (CMAKE_C_FLAGS_DEBUG          "-g")
SET (CMAKE_C_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
SET (CMAKE_C_FLAGS_RELEASE        "-O4 -DNDEBUG")
SET (CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g")

SET (CMAKE_CXX_COMPILER             "/usr/bin/clang++")
SET (CMAKE_CXX_FLAGS                "-std=c++20 -Wc++20-compat -pedantic -g -O3 -flto=thin -mllvm -inline-threshold=1000 -fexperimental-new-pass-manager -fno-strict-aliasing")

SET (CMAKE_CXX_FLAGS_DEBUG          "-g")
SET (CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
SET (CMAKE_CXX_FLAGS_RELEASE        "-O4 -DNDEBUG")
SET (CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")

SET (CMAKE_AR      "/usr/bin/llvm-ar")
SET (CMAKE_LINKER  "/usr/bin/llvm-ld")
SET (CMAKE_NM      "/usr/bin/llvm-nm")
SET (CMAKE_OBJDUMP "/usr/bin/llvm-objdump")
SET (CMAKE_RANLIB  "/usr/bin/llvm-ranlib")

include_directories(lib)

file(GLOB SOURCES "*.cpp" "*.h" "**/*.cpp" "**/*.h")
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lc++abi")

find_package(Threads REQUIRED)
find_package(Poco REQUIRED COMPONENTS Util Foundation Crypto Net NetSSL)
find_package(OpenSSL REQUIRED)

add_executable(daemon ${SOURCES})

target_link_libraries(daemon PRIVATE Threads::Threads)
target_link_libraries(daemon PUBLIC Poco::Util Poco::Foundation Poco::Net Poco::Crypto Poco::NetSSL)
