#include "ink_compiler.h"
#include "runtime/ink_story.h"

#include "ink_utils.h"

#include <iostream>
#include <format>

#define TEST_PATH(path) INKCPP_WORKING_DIR "/tests/" path

#define print(fmt, ...) std::cout << std::format(fmt __VA_OPT__(,) __VA_ARGS__) << std::endl

template <typename T>
void print_vector(const std::vector<T>& vector) {
	std::cout << "{";
	for (unsigned int i = 0; i < vector.size(); ++i) {
		std::cout << vector[i];
		if (i < vector.size() - 1) {
			std::cout << ",";
		}
	}

	std::cout << "}" << std::endl;
}

int main() {
	InkCompiler compiler;
	InkStory story = compiler.compile_file(TEST_PATH("17_functions/17c_function_side_effects.ink"));

	std::cout << story.continue_story() << std::endl;
	std::cout << story.continue_story() << std::endl;
	std::cout << story.continue_story() << std::endl;
	std::cout << story.continue_story() << std::endl;
}
