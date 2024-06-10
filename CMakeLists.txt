cmake_minimum_required(VERSION 3.0.0)

project(asio_test VERSION 0.1.0)

# set_property(TARGET asio_test PROPERTY CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(asio INTERFACE)
target_compile_options(asio INTERFACE -DASIO_STANDALONE -DASIO_NO_DEPRECATED)
target_include_directories(asio INTERFACE ${CMAKE_SOURCE_DIR}/asio-1.30.2/include)
target_link_libraries(asio INTERFACE pthread)

add_executable(asio_test asio_test.cpp)
target_link_libraries(asio_test PRIVATE asio)

add_executable(recv_asio recv_asio.cpp)
target_link_libraries(recv_asio PRIVATE asio)

add_executable(send_asio send_asio.cpp)
target_link_libraries(send_asio PRIVATE asio)


add_executable(send_asio2 send_asio2.cpp)
target_link_libraries(send_asio2 PRIVATE asio)
