cmake_minimum_required(VERSION 3.17)
project(pobr_project)
set(APP_TARGET pobr_app)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)

file(GLOB_RECURSE SOURCE_FILES
        "${PROJECT_SOURCE_DIR}/src/*.cc"
        "${PROJECT_SOURCE_DIR}/src/*.cpp")

file(GLOB_RECURSE HEADER_FILES
        "${PROJECT_SOURCE_DIR}/src/*.h"
        "${PROJECT_SOURCE_DIR}/src/*.hpp")

set(LIBRARIES
        ${OpenCV_LIBS}
        ${EXTRA_LIBS})

include_directories(${PROJECT_SOURCE_DIR}/src)

add_executable(${APP_TARGET} ${HEADER_FILES} ${SOURCE_FILES})

target_link_libraries(${APP_TARGET} ${LIBRARIES})
