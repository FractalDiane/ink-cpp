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
	/*InkCompiler compiler;
	compiler.compile_file_to_file(TEST_PATH("1_content/1b_multiline_text.ink"), INKCPP_WORKING_DIR "/" "test3.inkb");*/

	std::string infile = INKCPP_WORKING_DIR "/" "test3.inkb";
	InkStory story{infile};
	std::cout << story.continue_story() << std::endl;
	std::cout << story.continue_story() << std::endl;
	std::cout << story.continue_story() << std::endl;
	std::cout << story.continue_story() << std::endl;
}
