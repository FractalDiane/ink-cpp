#include "ink_compiler.h"
#include "runtime/ink_story.h"

#include "ink_utils.h"

#include <iostream>
#include <format>
#include <cstring>

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

#define TEST_FOLDER "ink-proof"
#define TEST_FILE "58_compare_divert_targets"
//#define TEST_FOLDER "22_misc_bugs"
//#define TEST_FILE "22l_choice_divert_scope"
//#define TEST_FOLDER "23_long_examples"
//#define TEST_FILE "23b_crime_scene"

int main(int argc, char* argv[]) {
	InkCompiler compiler;
	InkStory story = compiler.compile_file(INKCPP_WORKING_DIR "/tests/" TEST_FOLDER "/" TEST_FILE ".ink");
	std::cout << story.continue_story() << std::endl;
	story.choose_choice_index(0);
	std::cout << story.continue_story() << std::endl;
	story.choose_choice_index(1);
	std::cout << story.continue_story() << std::endl;
	story.choose_choice_index(1);
	std::cout << story.continue_story() << std::endl;
	story.choose_choice_index(1);
	std::cout << story.continue_story() << std::endl;
	story.choose_choice_index(1);
	std::cout << story.continue_story() << std::endl;
	story.choose_choice_index(2);
	std::cout << story.continue_story() << std::endl;
	story.choose_choice_index(0);
	std::cout << story.continue_story() << std::endl;
	std::cout << story.continue_story() << std::endl;
	story.choose_choice_index(1);
	std::cout << story.continue_story() << std::endl;
	story.choose_choice_index(0);
	std::cout << story.continue_story() << std::endl;
	story.choose_choice_index(2);
	std::cout << story.continue_story() << std::endl;
	story.choose_choice_index(0);
	std::cout << story.continue_story() << std::endl;
	story.choose_choice_index(0);
	std::cout << story.continue_story() << std::endl;
	story.choose_choice_index(1);
	std::cout << story.continue_story() << std::endl;
	story.choose_choice_index(0);
	std::cout << story.continue_story() << std::endl;

	/*story.choose_choice_index(1);
	std::cout << story.continue_story() << std::endl;
	std::cout << story.continue_story() << std::endl;
	story.choose_choice_index(0);
	std::cout << story.continue_story() << std::endl;*/
}
