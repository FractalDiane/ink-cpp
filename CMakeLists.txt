cmake_minimum_required(VERSION 3.20)
cmake_policy(SET CMP0135 NEW)

project(ink_cpp)
set(CMAKE_CXX_STANDARD 23)

set(SRC_UTIL
	src/ink_utils.cpp
	src/expression_parser/expression_parser.cpp
)

set(SRC_COMPILER src/ink_compiler.cpp)

set(SRC_RUNTIME 
	src/runtime/ink_story_data.cpp
	src/runtime/ink_story.cpp
	src/runtime/ink_story_state.cpp
	src/runtime/ink_story_tracking.cpp
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
	src/objects/ink_object_logic.cpp
	src/objects/ink_object_sequence.cpp
	src/objects/ink_object_tag.cpp
	src/objects/ink_object_text.cpp
)

set(CMAKE_BUILD_TYPE Debug)

################ GOOGLE TEST
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/f8d7d77c06936315286eb55f8de22cd23c188571.zip
)

if(WIN32)
	set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()

FetchContent_MakeAvailable(googletest)
################ GOOGLE TEST

add_library(ink_backend STATIC ${SRC_OBJECTS} ${SRC_RUNTIME} ${SRC_UTIL})

add_executable(inkc main_compiler.cpp ${SRC_COMPILER} ${SRC_UTIL})
add_dependencies(inkc ink_backend)
target_link_libraries(inkc PUBLIC ink_backend)

add_executable(ink main_runner.cpp ${SRC_COMPILER})
add_dependencies(ink ink_backend)
target_link_libraries(ink PUBLIC ink_backend)

#if(CMAKE_BUILD_TYPE EQUAL Debug)
	add_executable(ink_main main.cpp ${SRC_COMPILER} ${SRC_UTIL})
	add_dependencies(ink_main ink_backend)
	target_link_libraries(ink_main PUBLIC ink_backend)
#endif()

add_executable(tests tests.cpp ${SRC_COMPILER} ${SRC_UTIL})
add_dependencies(tests ink_backend)
target_link_libraries(tests PUBLIC ink_backend GTest::gtest_main)

add_compile_definitions(INKB_VERSION=0 INKCPP_WORKING_DIR="${PROJECT_SOURCE_DIR}")

include_directories(include)

if(MSVC)
	set(INK_COMPILE_OPTIONS /Zc:preprocessor /W4 /WX /wd4100)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	set(INK_COMPILE_OPTIONS -Wall -Wpedantic -Werror -Wno-unused-lambda-capture -Wno-unused-parameter -Wno-error=unused-private-field -Wno-reorder)
else()
	set(INK_COMPILE_OPTIONS -Wall -Wpedantic -Werror -Wno-unused-parameter -Wno-reorder -lm)
endif()

target_compile_options(ink_backend PUBLIC ${INK_COMPILE_OPTIONS})
target_compile_options(ink PUBLIC ${INK_COMPILE_OPTIONS})
target_compile_options(inkc PUBLIC ${INK_COMPILE_OPTIONS})
