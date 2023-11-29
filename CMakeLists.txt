cmake_minimum_required(VERSION 3.20)
cmake_policy(SET CMP0135 NEW)

project(ink_compiler)
set(CMAKE_CXX_STANDARD 20)

set(SRC_COMPILER src/ink_compiler.cpp)

set(SRC_RUNTIME 
	src/runtime/ink_story_data.cpp
	src/runtime/ink_story.cpp
)

set(SRC_OBJECTS
	src/objects/ink_object.cpp
	src/objects/ink_object_choice.cpp
	src/objects/ink_object_choicetextmix.cpp
	src/objects/ink_object_conditional.cpp
	src/objects/ink_object_divert.cpp
	src/objects/ink_object_globalvariable.cpp
	src/objects/ink_object_glue.cpp
	src/objects/ink_object_interpolation.cpp
	src/objects/ink_object_linebreak.cpp
	src/objects/ink_object_sequence.cpp
	src/objects/ink_object_tag.cpp
	src/objects/ink_object_text.cpp
)

set(CMAKE_BUILD_TYPE Debug)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/f8d7d77c06936315286eb55f8de22cd23c188571.zip
)

if(WIN32)
	set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()

FetchContent_MakeAvailable(googletest)

add_executable(ink_compiler main.cpp ${SRC_COMPILER} ${SRC_OBJECTS} ${SRC_RUNTIME})
add_executable(ink_tests tests.cpp ${SRC_COMPILER} ${SRC_OBJECTS} ${SRC_RUNTIME})

target_compile_definitions(ink_compiler PUBLIC INKB_VERSION=0)
target_link_libraries(ink_tests GTest::gtest_main)

include_directories(include)
