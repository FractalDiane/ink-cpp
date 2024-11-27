#include "runtime/ink_story.h"
#include "ink_compiler.h"

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
	InkStory story;
	if (infile.ends_with(".inkb")) {
		story = InkStory(infile);
	} else {
		InkCompiler compiler;
		InkCompileToStoryResult compile_result = compiler.compile_file(infile);
		if (compile_result.has_value()) {
			story = std::move(*compile_result);
		} else {
			print("Error: {}\n", compile_result.error().what());
			return 1;
		}
	}
	
	while (true) {
		while (story.can_continue()) {
			std::expected<std::string, std::string> result = story.continue_story();
			if (result.has_value()) {
				if (!result->empty()) {
					print("{}\n\n", *result);
				}
			} else {
				print("Error: {}\n", result.error());
				return 1;
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
