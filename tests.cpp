#include <gtest/gtest.h>

#include "ink_compiler.h"
#include "runtime/ink_story.h"

#define FIXTURE(name) class name : public testing::Test {\
protected:\
	InkCompiler compiler;\
}

#define STORY(path) InkStory story = compiler.compile_file(path)
#define EXPECT_TEXT(...) {\
		std::vector<std::string> seq = {__VA_ARGS__};\
		for (const std::string& text : seq) {\
			EXPECT_EQ(story.continue_story(), text);\
		}\
	}

///////////////////////////////////////////////////////////////////////////////////////////////////

FIXTURE(RuntimeTests);

TEST_F(RuntimeTests, Content_SingleLineText) {
	STORY("../tests/1_content/1a_text.ink");
	EXPECT_TEXT("Hello, world");
}

TEST_F(RuntimeTests, Content_MultiLineText) {
	STORY("../tests/1_content/1b_multiline_text.ink");
	EXPECT_TEXT("Hello, world", "Hello?", "Hello, are you there?");
}

TEST_F(RuntimeTests, Content_Comments) {
	STORY("../tests/1_content/1c_comments.ink");
	EXPECT_TEXT(R"("What do you make of this?" she asked.)", R"("I couldn't possibly comment," I replied.)");
}

TEST_F(RuntimeTests, Content_Tags) {
	STORY("../tests/1_content/1d_tags.ink");
	EXPECT_TEXT("A line of normal game-text.");

	std::vector<std::string> tags = {"three * test", "other tags", "right here", "colour it blue"};
	EXPECT_EQ(story.get_current_tags(), tags);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

int main() {
	testing::InitGoogleTest();
	return RUN_ALL_TESTS();
}
