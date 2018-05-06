cmake_minimum_required(VERSION 3.6)
project(road_segment_cpp)


set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/thirdparty/SQLiteCpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
INCLUDE_DIRECTORIES(
        /usr/local/Cellar/spdlog/0.16.3/include/
        /usr/local/Cellar/rapidjson/1.1.0/include/
        ${CMAKE_CURRENT_LIST_DIR}/thirdparty/SQLiteCpp/include )
set(SOURCE_FILES src/main.cpp src/RoadData.cpp src/RoadData.h src/platform.h)
add_executable(road_segment_cpp ${SOURCE_FILES})

target_link_libraries(road_segment_cpp
        SQLiteCpp
        sqlite3
        pthread
        dl
        )