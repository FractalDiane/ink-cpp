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
		for (const std::string& expected_text : seq) {\
			EXPECT_EQ(story.continue_story(), expected_text);\
		}\
	}

#define EXPECT_CHOICES(...) {\
		std::vector<std::string> expected_choices = {__VA_ARGS__};\
		EXPECT_EQ(story.get_current_choices(), expected_choices);\
	}

///////////////////////////////////////////////////////////////////////////////////////////////////

FIXTURE(ContentTests);
FIXTURE(ChoiceTests);

TEST_F(ContentTests, SingleLineText) {
	STORY("../tests/1_content/1a_text.ink");
	EXPECT_TEXT("Hello, world");
}

TEST_F(ContentTests, MultiLineText) {
	STORY("../tests/1_content/1b_multiline_text.ink");
	EXPECT_TEXT("Hello, world", "Hello?", "Hello, are you there?");
}

TEST_F(ContentTests, Comments) {
	STORY("../tests/1_content/1c_comments.ink");
	EXPECT_TEXT(R"("What do you make of this?" she asked.)", R"("I couldn't possibly comment," I replied.)");
}

TEST_F(ContentTests, Tags) {
	STORY("../tests/1_content/1d_tags.ink");
	EXPECT_TEXT("A line of normal game-text.");

	std::vector<std::string> expected_tags = {"three * test", "other tags", "right here", "colour it blue"};
	EXPECT_EQ(story.get_current_tags(), expected_tags);
}

TEST_F(ChoiceTests, SingleChoice) {
	STORY("../tests/2_choices/2a_choice.ink");
	EXPECT_TEXT("Hello world!");

	EXPECT_CHOICES("Hello back!");
	story.choose_choice_index(0);
	EXPECT_TEXT("Hello back!", "Nice to hear from you!");
}

TEST_F(ChoiceTests, SuppressedChoiceText) {
	STORY("../tests/2_choices/2b_choice_suppressed.ink");
	EXPECT_TEXT("Hello world!");

	EXPECT_CHOICES("Hello back!");
	story.choose_choice_index(0);
	EXPECT_TEXT("Nice to hear from you!");
}

TEST_F(ChoiceTests, MixedChoiceText) {
	STORY("../tests/2_choices/2c_choice_mixed.ink");
	EXPECT_TEXT("Hello world!");

	EXPECT_CHOICES("Hello back!");
	story.choose_choice_index(0);
	EXPECT_TEXT("Hello right back to you!", "Nice to hear from you!");
}

TEST_F(ChoiceTests, MultipleChoices) {
	std::vector<std::vector<std::string>> expected_texts = {
		{R"("I am somewhat tired," I repeated.)", R"("Really," he responded. "How deleterious.")"},
		{R"("Nothing, Monsieur!" I replied.)", R"("Very good, then.")"},
		{R"("I said, this journey is appalling and I want no more of it.")", R"("Ah," he replied, not unkindly. "I see you are feeling frustrated. Tomorrow, things will improve.")"},
	};

	for (int i = 0; i < 3; ++i) {
		STORY("../tests/2_choices/2d_choice_multiple.ink");
		EXPECT_TEXT(R"("What's that?" my master asked.)");

		EXPECT_CHOICES(R"("I am somewhat tired.")", R"("Nothing, Monsieur!")", R"("I said, this journey is appalling.")");
		story.choose_choice_index(i);
		EXPECT_TEXT(expected_texts[i][0], expected_texts[i][1]);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

int main() {
	testing::InitGoogleTest();
	return RUN_ALL_TESTS();
}
