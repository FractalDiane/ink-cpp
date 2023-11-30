#include "ink_compiler.h"
#include "runtime/ink_story.h"

#include <iostream>

int main() {
	/*std::string story = R"(Once upon a time...

 * There were two choices.
 * There were four lines of content.

- They lived happily ever after.
    -> END
)";*/

	InkCompiler compiler;
	InkStory story = compiler.compile_file("../tests/2_choices/2a_choice.ink");
	//story.print_info();
	//compiler.save_data_to_file(story.get_story_data(), "../test.inkb");

	std::cout << story.continue_story() << std::endl;
	story.choose_choice_index(0);
	std::cout << story.continue_story() << std::endl;
	std::cout << story.continue_story() << std::endl;

	//std::cout << "=====================================" << std::endl;

	/*InkStory story2{"../test.inkb"};
	//story2.print_info();
	std::cout << story2.continue_story() << std::endl;
	std::cout << story2.continue_story() << std::endl;*/
}
