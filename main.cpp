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
	InkStory story = compiler.compile_file("../test.ink");
	story.print_info();
}
