#include "ink_compiler.h"

#include <iostream>

int main() {
	std::string story = R"(Once upon a time...

 * There were two choices.
 * There were four lines of content.

- They lived happily ever after.
    -> END
)";

	InkCompiler compiler;
	compiler.compile_file("../test.ink");
}
