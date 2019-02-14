cmake_minimum_required(VERSION 2.6)

set(CMAKE_CXX_STANDARD 14)

set(HAS_LIBPS3EYE 1 PARENT_SCOPE)
message("setting HAS_LIBPS3EYE")

# --- dependencies ---

if (NOT HAS_LIBGG)
	message("including libgg")
	add_subdirectory(../../../libgg libgg)
endif (NOT HAS_LIBGG)
if (NOT HAS_FRAMEWORK)
	message("including framework")
	add_subdirectory(../../../framework framework)
endif (NOT HAS_FRAMEWORK)

#

project(libps3eye)

# --- libps3eye ---

file(GLOB_RECURSE source "../src/*.*")

add_library(libps3eye ${source})

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules)

find_package(PkgConfig)
find_package(LibUSB REQUIRED)

target_include_directories(libps3eye PUBLIC "${PROJECT_SOURCE_DIR}/../src")
target_include_directories(libps3eye PRIVATE "${PROJECT_SOURCE_DIR}" "${LibUSB_INCLUDE_DIRS}")

target_link_libraries(libps3eye PRIVATE libgg ${LibUSB_LIBRARIES})

#

add_executable(example main.cpp)

target_link_libraries(example PRIVATE libps3eye framework ${FRAMEWORK_LIBRARIES})
