cmake_minimum_required(VERSION 3.16)
project(sm3)

set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mavx" )
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mavx2" )
find_package(OpenSSL REQUIRED)
add_executable(sm3 sm3.h main.c openssl_sm3.c simple_sm3.c opti_sm3.c)

set(DEP_LIBS
    OpenSSL::Crypto OpenSSL::SSL  # modern
)

target_link_libraries(sm3 ${DEP_LIBS})
