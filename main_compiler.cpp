#include "ink_compiler.h"

#if __has_include(<print>)
#include <print>
using std::print;
#else
#include <format>
#include <iostream>
#define print(fmt, ...) std::cout << std::format(fmt __VA_OPT__(,) __VA_ARGS__)
#endif

int main(int argc, char* argv[]) {
	if (argc < 2 || argc > 3) {
		print("Error: No ink file specified\n");
		return 1;
	}

	std::string infile = argv[1];
	std::string noext = infile.substr(0, infile.find('.'));

	InkCompiler compiler;
	InkCompileToFileResult compile_result = compiler.compile_file_to_file(infile, argc == 3 ? argv[2] : noext + ".inkb");
	if (!compile_result.has_value()) {
		print("Error: {}", compile_result.error().what());
		return 1;
	}
}
