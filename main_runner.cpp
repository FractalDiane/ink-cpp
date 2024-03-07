#include "runtime/ink_story.h"
#include "ink_compiler.h"

#if __has_include(<print>)
#include <print>
using std::print;
#else
#include <format>
#define print(fmt, ...) std::cout << std::format(fmt __VA_OPT__(,) __VA_ARGS__)
#endif

#include <iostream>

int main(int argc, char* argv[]) {
	if (argc != 2) {
		print("Error: No ink file specified\n");
		return 1;
	}

	std::string infile = argv[1];

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
			for (std::size_t i = 0; i < current_choices.size(); ++i) {
				print("{}: {}\n", i + 1, current_choices[i]);
			}

			print("\n");

			std::size_t choice = SIZE_MAX;
			while (choice > current_choices.size()) {
				print("> ");
				std::cin >> choice;
			}

			story.choose_choice_index(static_cast<std::size_t>(choice - 1));
		} else {
			break;
		}
	}
}
