cmake_minimum_required(VERSION 2.8)
project(backEnd)
add_executable(${PROJECT_NAME}
    "main.cpp"
    "backend.cpp"
    "opcuaServer.cpp"
    "websocketServer.cpp"
    "sqlClient.cpp"
    "util.cpp")

target_link_libraries(${PROJECT_NAME} "boost_system" "pthread" "mariadbclient"                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               "open62541")

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O3 -Wall ")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
