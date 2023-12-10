#include "ink_compiler.h"

#if __has_include(<print>)
#include <print>
using std::print;
#else
#include <format>
#define print(fmt, ...) std::cout << std::format(fmt __VA_OPT__(,) __VA_ARGS__) << std::endl
#endif

int main(int argc, char* argv[]) {
	
	
}
