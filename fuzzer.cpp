#include "expression_parser/expression_parser.h"

#include <string>
#include <print>

#include <random>

static const std::string expression_chars = "0123456789+-*/%^\"()!&|? ";

// failed but shouldn't
//  "698("^58

int main(int argc, char* argv[]) {
	if (argc == 2) {
		std::println("Expression: {}", argv[1]);
		try {
			ExpressionParserV2::StoryVariableInfo blank_var_info;
			static_cast<void>(ExpressionParserV2::tokenize_and_shunt_expression(argv[1], blank_var_info, ExpressionParserV2::ContentsAllowed::Any));
			std::println("Result: Compiled successfully\n");
		} catch (const ExpressionParserV2::ExpressionException& e) {
			std::println("Result: {}\n", e.what());
		}
	} else {
		std::mt19937 rng{std::random_device()()};
		std::uniform_int_distribution<std::size_t> rand_char{0, expression_chars.length() - 1};
		std::normal_distribution<float> rand_range{10, 2};
		for (int i = 0; i < 10; ++i) {
			std::string result;
			std::size_t length = static_cast<std::size_t>(rand_range(rng));
			result.reserve(length);
			for (int j = 0; j < length; ++j) {
				result.push_back(expression_chars[rand_char(rng)]);
			}

			std::println("Expression: {}", result);
			try {
				ExpressionParserV2::StoryVariableInfo blank_var_info;
				static_cast<void>(ExpressionParserV2::tokenize_and_shunt_expression(result, blank_var_info, ExpressionParserV2::ContentsAllowed::Any));
				std::println("Result: Compiled successfully\n");
			} catch (const ExpressionParserV2::ExpressionException& e) {
				std::println("Result: {}\n", e.what());
			}
		}
	}
}
