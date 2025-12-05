cmake_minimum_required(VERSION 3.16)
project(cppp C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -D_FILE_OFFSET_BITS=64 -fsanitize=address -g")

find_package(OpenSSL REQUIRED)

add_executable(cppp
	src/cppp.c
	src/include/print_warns.c
	src/include/cli_parser.c
	src/include/copy.c
	src/include/file_info.c
	src/include/progress_bar.c
	src/include/sha256.c
	src/include/init_process.c
)

target_link_libraries(cppp PRIVATE OpenSSL::Crypto)