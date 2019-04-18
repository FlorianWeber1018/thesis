cmake_minimum_required(VERSION 2.8)

project(backEnd)
add_executable(${PROJECT_NAME} "main.cpp" "backend.cpp" "opcuaServer.cpp" "websocketServer.cpp" "sqlClient.cpp")
target_link_libraries(${PROJECT_NAME} "boost_system" "pthread" "mariadbclient")



set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O3 -Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
