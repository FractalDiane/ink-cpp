#include "runtime/ink_story.h"
#include "ink_compiler.h"

#include "builtin-features.inc"

#include <iostream>
#if __has_include(<print>)
#include <print>
using std::print;
#else
#include <format>
#define print(fmt, ...) std::cout << std::format(fmt __VA_OPT__(,) __VA_ARGS__)
#endif

int main(int argc, char* argv[]) {
	if (argc != 2) {
		print("Error: No ink file specified\n");
		return 1;
	}

	std::string infile = argv[1];

	cparse_startup();

	InkCompiler compiler;
	InkStory story = compiler.compile_file(infile);
	while (true) {
		while (story.can_continue()) {
			std::string result = story.continue_story();
			if (!result.empty()) {
				print("{}\n\n", result);
			}
		}

		const std::vector<std::string>& current_choices = story.get_current_choices();
		if (!current_choices.empty()) {
			for (int i = 0; i < current_choices.size(); ++i) {
				print("{}: {}\n", i + 1, current_choices[i]);
			}

			print("\n");

			int choice = -1;
			while (choice <= 0 || choice > current_choices.size()) {
				print("> ");
				std::cin >> choice;
			}

			story.choose_choice_index(choice - 1);
		} else {
			break;
		}
	}
}
