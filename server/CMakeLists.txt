cmake_minimum_required(VERSION 3.29)

if (DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
else ()
    message(FATAL_ERROR "Please define VCPKG_ROOT to point to the vcpkg directory")
endif ()

project(ws_yolo_server)

set(VCPKG_MANIFEST_MODE ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(OpenCV REQUIRED)
find_package(ZLIB REQUIRED)
find_path(UWEBSOCKETS_INCLUDE_DIRS "uwebsockets/App.h")
if (NOT UWEBSOCKETS_INCLUDE_DIRS)
    message(FATAL_ERROR "Could not find uWebSockets headers")
endif ()
find_library(USOCKETS_LIB uSockets)
find_package(OpenSSL REQUIRED)
find_package(libuv CONFIG REQUIRED)

# Include OpenCV and uWebSockets headers
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${UWEBSOCKETS_INCLUDE_DIRS})

# Source files
set(SOURCES src/main.cpp src/detector.cpp src/ws_server.cpp)

# Header files
set(HEADERS include/detector.h include/ws_server.h)

add_executable(ws_yolo_server ${SOURCES} ${HEADERS})

target_link_libraries(ws_yolo_server PRIVATE ${OpenCV_LIBS} ${USOCKETS_LIB} ZLIB::ZLIB OpenSSL::SSL OpenSSL::Crypto $<IF:$<TARGET_EXISTS:libuv::uv_a>,libuv::uv_a,libuv::uv>)
target_include_directories(ws_yolo_server PRIVATE ${CMAKE_SOURCE_DIR}/include ${UWEBSOCKETS_INCLUDE_DIRS})
