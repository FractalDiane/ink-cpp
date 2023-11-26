cmake_minimum_required(VERSION 3.20)

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

add_executable(ink_compiler main.cpp ${SRC_COMPILER} ${SRC_OBJECTS} ${SRC_RUNTIME})

include_directories(include include/objects)
