cmake_minimum_required(VERSION 3.13.0)
project(armulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_SUPPRESS_REGENERATION true)

add_subdirectory(core2)

include_directories(include)
include_directories(core2/include)

file(GLOB_RECURSE armulatorSrc
	"include/*.hpp"
	"src/*.cpp"
)

add_library(
	armulator STATIC
	${armulatorSrc}
)

target_link_libraries(armulator ocore)

if(MSVC)
    target_compile_options(armulator PRIVATE /W4 /WX /MD /MP /wd4201 /Ob2)
else()
    target_compile_options(armulator PRIVATE -Wall -Wextra -pedantic -Werror)
endif()