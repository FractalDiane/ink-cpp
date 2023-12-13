#include "ink_compiler.h"
#include "runtime/ink_story.h"

#include "ink_utils.h"
//#include "exprtk/exprtk.hpp"
#include "shunting-yard.h"
#include "builtin-features.inc"

#include <iostream>
#include <format>

#ifdef _WIN32
#define TEST_PATH(path) R"(C:/Users/Duncan Sparks/Desktop/Programming/ink-cpp/tests/)" path
#else
#define TEST_PATH(path) R"(/home/diane/Programming/ink-cpp/tests/)" path
#endif

#define print(fmt, ...) std::cout << std::format(fmt __VA_OPT__(,) __VA_ARGS__) << std::endl

cparse::packToken test_func(cparse::TokenMap scope) {
	return scope["a"].asInt() + scope["b"].asInt();
}

int main() {
	cparse_startup();

	InkCompiler compiler;
	InkStory story = compiler.compile_file(R"(C:\Users\Duncan Sparks\Desktop\Programming\ink-cpp\tests\10_gathers\10b_multi_gather.ink)");

	std::cout << story.continue_story() << std::endl;
	story.choose_choice_index(1);
	std::cout << story.continue_story() << std::endl;
	story.choose_choice_index(1);
	std::cout << story.continue_story() << std::endl;
	std::cout << story.continue_story() << std::endl;
	std::cout << story.continue_story() << std::endl;
	std::cout << story.continue_story() << std::endl;
}
