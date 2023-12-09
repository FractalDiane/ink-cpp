#include "ink_compiler.h"
#include "runtime/ink_story.h"

#include "ink_utils.h"
//#include "exprtk/exprtk.hpp"

#include <iostream>
#include <format>

#ifdef _WIN32
#define TEST_PATH(path) R"(C:/Users/Duncan Sparks/Desktop/Programming/ink-cpp/tests/)" path
#else
#define TEST_PATH(path) R"(/home/diane/Programming/ink-cpp/tests/)" path
#endif

#define print(fmt, ...) std::cout << std::format(fmt __VA_OPT__(,) __VA_ARGS__) << std::endl

int main() {
	/*std::string story = R"(Once upon a time...

 * There were two choices.
 * There were four lines of content.

- They lived happily ever after.
    -> END
)";*/

	/*exprtk::expression<double> expression;
	exprtk::symbol_table<double> symbol_table;
	exprtk::parser<double> parser;

	symbol_table.add_constant("main.s3", 5);
	expression.register_symbol_table(symbol_table);
	parser.compile("main.s3 > 2", expression);
	std::cout << expression.value() << std::endl;*/

	//std::string test = strip_string_edges("\t");

	InkCompiler compiler;
	InkStory story = compiler.compile_file(TEST_PATH("6_stitches/6a_stitches.ink"));
	//story.print_info();
	//compiler.save_data_to_file(story.get_story_data(), "../test.inkb");

	std::cout << story.continue_story() << std::endl;
	story.choose_choice_index(1);
	std::cout << story.continue_story() << std::endl;
	/*story.choose_choice_index(0);
	std::cout << story.continue_story() << std::endl;
	story.choose_choice_index(0);
	std::cout << story.continue_story() << std::endl;
	story.choose_choice_index(0);
	std::cout << story.continue_story() << std::endl;*/
	/*story.choose_choice_index(0);
	std::cout << story.continue_story() << std::endl;
	story.choose_choice_index(1);
	std::cout << story.continue_story() << std::endl;
	std::cout << story.continue_story() << std::endl;*/
	
	//std::cout << story.continue_story() << std::endl;

	/*for (int i = 0; i < 3; ++i) {
		InkCompiler compiler;
		InkStory story = compiler.compile_file(TEST_PATH("2_choices/2d_choice_multiple.ink"));
		EXPECT_TEXT(R"("What's that?" my master asked.)");

		EXPECT_CHOICES(R"("I am somewhat tired.")", R"("Nothing, Monsieur!")", R"("I said, this journey is appalling.")");
		story.choose_choice_index(i);
		EXPECT_TEXT(expected_texts[i][0], expected_texts[i][1]);
	}*/

	//std::cout << "=====================================" << std::endl;

	/*InkStory story2{"../test.inkb"};
	//story2.print_info();
	std::cout << story2.continue_story() << std::endl;
	std::cout << story2.continue_story() << std::endl;*/
}
