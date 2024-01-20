#include "ink_compiler.h"
#include "runtime/ink_story.h"

#include "ink_utils.h"
//#include "exprtk/exprtk.hpp"
#include "shunting-yard.h"
#include "builtin-features.inc"
#include "ink_cparse_ext.h"

#include <iostream>
#include <format>

#define TEST_PATH(path) INKCPP_WORKING_DIR "/tests/" path

#define print(fmt, ...) std::cout << std::format(fmt __VA_OPT__(,) __VA_ARGS__) << std::endl

int main() {
	cparse_startup();
	InkCparseStartup ink_startup;
	InkCparseStartupParser ink_startup_parser;
	//std::cout << INKCPP_WORKING_DIR << std::endl;

	InkCompiler compiler;
	InkStory story = compiler.compile_file(TEST_PATH("14_logic/14a_basic_logic.ink"));

	std::cout << story.continue_story() << std::endl;
	//std::cout << story.continue_story() << std::endl;
	//story.choose_choice_index(1);
	//std::cout << story.continue_story() << std::endl;
	//std::cout << story.continue_story() << std::endl;
	//std::cout << story.continue_story() << std::endl;
	//std::cout << story.continue_story() << std::endl;
	//story.choose_choice_index(0);
	//std::cout << story.continue_story() << std::endl;
	//std::cout << story.continue_story() << std::endl;
	//story.choose_choice_index(0);
	//std::cout << story.continue_story() << std::endl;
	//story.choose_choice_index(0);
	//std::cout << story.continue_story() << std::endl;
	/*story.choose_choice_index(0);
	std::cout << story.continue_story() << std::endl;
	story.choose_choice_index(0);
	std::cout << story.continue_story() << std::endl;
	story.choose_choice_index(0);
	std::cout << story.continue_story() << std::endl;
	std::cout << story.continue_story() << std::endl;*/
}
