#include "ink_compiler.h"
#include "runtime/ink_story.h"

#include "ink_utils.h"
//#include "exprtk/exprtk.hpp"
#include "shunting-yard.h"
#include "builtin-features.inc"

#include <iostream>
#include <format>

#define TEST_PATH(path) INKCPP_WORKING_DIR "/tests/" path

#define print(fmt, ...) std::cout << std::format(fmt __VA_OPT__(,) __VA_ARGS__) << std::endl

int main() {
	cparse_startup();
	//std::cout << INKCPP_WORKING_DIR << std::endl;

	InkCompiler compiler;
	InkStory story = compiler.compile_file(TEST_PATH("13_global_variables/13a_variable_checks.ink"));

	story.set_variable("mood", 1);
	story.set_variable("knows_about_wager", true);

	std::cout << story.continue_story() << std::endl;
	story.choose_choice_index(0);
	std::cout << story.continue_story() << std::endl;
	std::cout << story.continue_story() << std::endl;
	story.choose_choice_index(2);
	std::cout << story.continue_story() << std::endl;
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
