#include <gtest/gtest.h>

#include "ink_compiler.h"
#include "runtime/ink_story.h"

class RuntimeTest : public testing::Test {
protected:
	InkCompiler compiler;
};

#define STORY(path) InkStory story = compiler.compile_file(path)
#define EXPECT_TEXT(...) {\
		std::vector<std::string> seq = {__VA_ARGS__};\
		for (const std::string& text : seq) {\
			EXPECT_EQ(story.continue_story(), text);\
		}\
	}

TEST_F(RuntimeTest, Content_SingleLineText) {
	STORY("../tests/1_content/1a_text.ink");
	EXPECT_TEXT("Hello, world");
}

TEST_F(RuntimeTest, Content_MultiLineText) {
	STORY("../tests/1_content/1b_multiline_text.ink");
	EXPECT_TEXT("Hello, world", "Hello?", "Hello, are you there?");
}

///////////////////////////////////////////////////////////////////////////////////////////////////

int main() {
	testing::InitGoogleTest();
	return RUN_ALL_TESTS();
}
