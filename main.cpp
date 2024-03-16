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

#define TEST_FOLDER "17_functions"
#define TEST_FILE "17a_functions_in_logic"

int main(int argc, char* argv[]) {
	//if (argc > 1) {
	//	if (!strcmp(argv[1], "build") || !strcmp(argv[1], "compile")) {
	//		InkCompiler compiler;
	//		compiler.compile_file_to_file(INKCPP_WORKING_DIR "/tests/" TEST_FOLDER "/" TEST_FILE ".ink", INKCPP_WORKING_DIR "/" TEST_FILE ".inkb");
	//	} else if (!strcmp(argv[1], "run")) {
			std::string infile = INKCPP_WORKING_DIR "/" TEST_FILE ".inkb";
			InkStory story{infile};
			while (story.can_continue()) {
				std::cout << story.continue_story_maximally() << std::endl;
				if (!story.get_current_choices().empty()) {
					story.choose_choice_index(0);
				}
			}
		//}
	//}
}
