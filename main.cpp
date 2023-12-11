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

	std::string script = R"(It's been {TURNS_SINCE(-> main)} turns now
-> main
=== main ===
HELLO THERE
It's been {TURNS_SINCE(-> main)} turns now
* [1] -> two
* [2] -> two
* [3] -> two

=== two ===
It's been {TURNS_SINCE(-> main)} turn now)";

	InkCompiler compiler;
	InkStory story = compiler.compile_script(script);

	std::cout << story.continue_story() << std::endl;
	std::cout << story.continue_story() << std::endl;
	std::cout << story.continue_story() << std::endl;
	story.choose_choice_index(0);
	std::cout << story.continue_story() << std::endl;
	std::cout << story.continue_story() << std::endl;
}
