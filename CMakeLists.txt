cmake_minimum_required(VERSION 3.5)
project(road_segment_cpp)


set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/thirdparty/SQLiteCpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
INCLUDE_DIRECTORIES(
        ${CMAKE_CURRENT_LIST_DIR}/thirdparty/spdlog/include
        ${CMAKE_CURRENT_LIST_DIR}/thirdparty/rapidjson/include
        ${CMAKE_CURRENT_LIST_DIR}/thirdparty/tqdm.cpp/include
        ${CMAKE_CURRENT_LIST_DIR}/thirdparty/SQLiteCpp/include )
set(SOURCE_FILES src/main.cpp src/RoadData.cpp src/RoadData.h src/platform.h src/SimpleMatcher.cpp src/SimpleMatcher.h)
add_executable(road_segment_cpp ${SOURCE_FILES})

target_link_libraries(road_segment_cpp
        SQLiteCpp
        sqlite3
        pthread
        dl
        )