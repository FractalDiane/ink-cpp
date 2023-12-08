#include <gtest/gtest.h>

#include "ink_compiler.h"
#include "runtime/ink_story.h"

#define FIXTURE(name) class name : public testing::Test {\
protected:\
	InkCompiler compiler;\
}

#ifdef _WIN32
#define STORY(path) InkStory story = compiler.compile_file(R"(C:/Users/Duncan Sparks/Desktop/Programming/ink-cpp/tests/)" path)
#else
#define STORY(path) InkStory story = compiler.compile_file(R"(/home/diane/Programming/ink-cpp/tests/)" path)
#endif


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
FIXTURE(KnotTests);
FIXTURE(DivertTests);
FIXTURE(BranchingTests);
FIXTURE(StitchTests);
FIXTURE(VaryingChoiceTests);
FIXTURE(VariableTextTests);

TEST_F(ContentTests, SingleLineText) {
	STORY("1_content/1a_text.ink");
	EXPECT_TEXT("Hello, world");
}

TEST_F(ContentTests, MultiLineText) {
	STORY("1_content/1b_multiline_text.ink");
	EXPECT_TEXT("Hello, world", "Hello?", "Hello, are you there?");
}

TEST_F(ContentTests, Comments) {
	STORY("1_content/1c_comments.ink");
	EXPECT_TEXT(R"("What do you make of this?" she asked.)", R"("I couldn't possibly comment," I replied.)");
}

TEST_F(ContentTests, Tags) {
	STORY("1_content/1d_tags.ink");
	EXPECT_TEXT("A line of normal game-text.");

	std::vector<std::string> expected_tags = {"three * test", "other tags", "right here", "colour it blue"};
	EXPECT_EQ(story.get_current_tags(), expected_tags);
}

TEST_F(ChoiceTests, SingleChoice) {
	STORY("2_choices/2a_choice.ink");
	EXPECT_TEXT("Hello world!");

	EXPECT_CHOICES("Hello back!");
	story.choose_choice_index(0);
	EXPECT_TEXT("Hello back!", "Nice to hear from you!");
}

TEST_F(ChoiceTests, SuppressedChoiceText) {
	STORY("2_choices/2b_choice_suppressed.ink");
	EXPECT_TEXT("Hello world!");

	EXPECT_CHOICES("Hello back!");
	story.choose_choice_index(0);
	EXPECT_TEXT("Nice to hear from you!");
}

TEST_F(ChoiceTests, MixedChoiceText) {
	STORY("2_choices/2c_choice_mixed.ink");
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
		STORY("2_choices/2d_choice_multiple.ink");
		EXPECT_TEXT(R"("What's that?" my master asked.)");

		EXPECT_CHOICES(R"("I am somewhat tired.")", R"("Nothing, Monsieur!")", R"("I said, this journey is appalling.")");
		story.choose_choice_index(i);
		EXPECT_TEXT(expected_texts[i][0], expected_texts[i][1]);
	}
}

TEST_F(KnotTests, Knots) {
	STORY("3_knots/3a_knot.ink");
	EXPECT_TEXT("Hello world");
}

TEST_F(DivertTests, Diverts) {
	STORY("4_diverts/4a_divert.ink");
	EXPECT_TEXT("We hurried home to Savile Row as fast as we could.");
}

TEST_F(DivertTests, DivertsWithGlue) {
	STORY("4_diverts/4b_glue.ink");
	EXPECT_TEXT("We hurried home to Savile Row as fast as we could.");
}

TEST_F(BranchingTests, AdvancedBranching) {
	std::vector<std::string> expected_texts = {
		"You open the gate, and step out onto the path.",
		"You smash down the gate, and charge out onto the path.",
		"You turn back and go home. Pitiful.",
	};

	for (int i = 0; i < 3; ++i) {
		STORY("5_branching/5a_branch.ink");
		EXPECT_TEXT("You stand by the wall of Analand, sword in hand.");

		EXPECT_CHOICES("Open the gate", "Smash down the gate", "Turn back and go home");
		story.choose_choice_index(i);
		EXPECT_TEXT(expected_texts[i]);
	}
}

TEST_F(BranchingTests, BranchingAndJoining) {
	std::vector<std::vector<std::string>> expected_texts = {
		{R"("There is not a moment to lose!" I declared.)", "We hurried home to Savile Row as fast as we could."},
		{R"("Monsieur, let us savour this moment!" I declared.)", "My master clouted me firmly around the head and dragged me out of the door.", "He insisted that we hurried home to Savile Row as fast as we could."},
		{"We hurried home to Savile Row as fast as we could."},
	};

	for (int i = 0; i < 3; ++i) {
		STORY("5_branching/5b_branch_and_join.ink");
		EXPECT_TEXT("We arrived into London at 9.45pm exactly.");

		EXPECT_CHOICES(R"("There is not a moment to lose!")", R"("Monsieur, let us savour this moment!")", R"(We hurried home)");
		story.choose_choice_index(i);
		for (const std::string& expected_text : expected_texts[i]) {
			EXPECT_EQ(story.continue_story(), expected_text);
		}
	}
}

TEST_F(StitchTests, BasicStitches) {
	std::vector<std::string> expected_texts = {
		"Welcome to first class, sir.",
		"Welcome to third class, sir.",
		"You're not supposed to be here.",
	};

	for (int i = 0; i < 3; ++i) {
		STORY("6_stitches/6a_stitches.ink");
		story.continue_story();
		EXPECT_CHOICES("Travel in first class", "Travel in third class", "Travel in the guard's van");

		story.choose_choice_index(i);
		EXPECT_TEXT(expected_texts[i]);
	}
}

// TODO: Expect compilation fail
/*TEST_F(StitchTests, StitchesWithoutFallthrough) {
	STORY("6_stitches/6b_stitches_no_fallthrough.ink");
	EXPECT_TEXT("You are on a train. Whoa?", "");
	
}*/

TEST_F(StitchTests, StitchesWithLocalDiverts) {
	STORY("6_stitches/6c_stitches_local_divert.ink");
	EXPECT_TEXT("I settled my master.");
	EXPECT_CHOICES("Move to third class");
	story.choose_choice_index(0);
	EXPECT_TEXT("I put myself in third.");
}

TEST_F(VaryingChoiceTests, ChoicesBeingUsedUp) {
	STORY("7_varying/7a_fallback_choice.ink");
	EXPECT_TEXT("You search desperately for a friendly face in the crowd.");
	EXPECT_CHOICES("The woman in the hat?", "The man with the briefcase?");

	story.choose_choice_index(0);
	EXPECT_TEXT("The woman in the hat pushes you roughly aside. You search desperately for a friendly face in the crowd.");
	EXPECT_CHOICES("The man with the briefcase?");
}

TEST_F(VaryingChoiceTests, FallbackChoices) {
	STORY("7_varying/7a_fallback_choice.ink");
	EXPECT_TEXT("You search desperately for a friendly face in the crowd.");
	EXPECT_CHOICES("The woman in the hat?", "The man with the briefcase?");

	story.choose_choice_index(1);
	EXPECT_TEXT("The man with the briefcase looks disgusted as you stumble past him. You search desperately for a friendly face in the crowd.");
	EXPECT_CHOICES("The woman in the hat?");
	story.choose_choice_index(0);
	EXPECT_TEXT("The woman in the hat pushes you roughly aside. You search desperately for a friendly face in the crowd.");
	EXPECT_TEXT("But it is too late: you collapse onto the station platform. This is the end.");
}

TEST_F(VaryingChoiceTests, StickyChoices) {
	STORY("7_varying/7b_sticky_choice.ink");
	EXPECT_TEXT("You are on the couch.", "");
	EXPECT_CHOICES("Eat another donut", "Get off the couch");

	story.choose_choice_index(0);
	EXPECT_TEXT("You eat another donut.");
	EXPECT_CHOICES("Eat another donut", "Get off the couch");
	story.choose_choice_index(0);
	EXPECT_TEXT("You eat another donut.");
	EXPECT_CHOICES("Eat another donut", "Get off the couch");
	story.choose_choice_index(1);
	EXPECT_TEXT("You struggle up off the couch to go and compose epic poetry.");
	EXPECT_CHOICES("Eat another donut");
}

TEST_F(VaryingChoiceTests, ConditionalChoices) {
	STORY("7_varying/7c_conditional_choice.ink");
	EXPECT_TEXT("");
	EXPECT_CHOICES("Choice 1", "Choice 2");

	story.choose_choice_index(1);
	EXPECT_TEXT("Hello 2", "Hello 3", "");
	EXPECT_CHOICES("Choice 1", "Choice 2", "Choice 3");
}

TEST_F(VariableTextTests, Sequences) {
	STORY("8_variable_text/8a_sequence.ink");
	EXPECT_TEXT("I bought a coffee with my five-pound note.");
	EXPECT_CHOICES("Buy another one");
	story.choose_choice_index(0);
	EXPECT_TEXT("I bought a second coffee for my friend.");
	EXPECT_CHOICES("Buy another one");
	story.choose_choice_index(0);
	EXPECT_TEXT("I didn't have enough money to buy any more coffee.");
	EXPECT_CHOICES("Buy another one");
	story.choose_choice_index(0);
	EXPECT_TEXT("I didn't have enough money to buy any more coffee.");
}

TEST_F(VariableTextTests, Cycles) {
	STORY("8_variable_text/8b_cycle.ink");
	EXPECT_TEXT("It was Monday today.");
	EXPECT_CHOICES("Tomorrow?");
	story.choose_choice_index(0);
	EXPECT_TEXT("It was Tuesday today.");
	EXPECT_CHOICES("Tomorrow?");
	story.choose_choice_index(0);
	EXPECT_TEXT("It was Wednesday today.");
	EXPECT_CHOICES("Tomorrow?");
	story.choose_choice_index(0);
	EXPECT_TEXT("It was Thursday today.");
	EXPECT_CHOICES("Tomorrow?");
	story.choose_choice_index(0);
	EXPECT_TEXT("It was Friday today.");
	EXPECT_CHOICES("Tomorrow?");
	story.choose_choice_index(0);
	EXPECT_TEXT("It was Saturday today.");
	EXPECT_CHOICES("Tomorrow?");
	story.choose_choice_index(0);
	EXPECT_TEXT("It was Sunday today.");
	EXPECT_CHOICES("Tomorrow?");
	story.choose_choice_index(0);
	EXPECT_TEXT("It was Monday today.");
	EXPECT_CHOICES("Tomorrow?");
	story.choose_choice_index(0);
	EXPECT_TEXT("It was Tuesday today.");
	EXPECT_CHOICES("Tomorrow?");
	story.choose_choice_index(0);
	EXPECT_TEXT("It was Wednesday today.");
	EXPECT_CHOICES("Tomorrow?");
	story.choose_choice_index(0);
	EXPECT_TEXT("It was Thursday today.");
	EXPECT_CHOICES("Tomorrow?");
	story.choose_choice_index(0);
	EXPECT_TEXT("It was Friday today.");
}

TEST_F(VariableTextTests, OnceOnly) {
	STORY("8_variable_text/8c_onceonly.ink");
	EXPECT_TEXT("He told me a joke. I laughed politely.");
	story.choose_choice_index(0);
	EXPECT_TEXT("He told me a joke. I smiled.");
	story.choose_choice_index(0);
	EXPECT_TEXT("He told me a joke. I grimaced.");
	story.choose_choice_index(0);
	EXPECT_TEXT("He told me a joke. I promised myself to not react again.");
	story.choose_choice_index(0);
	EXPECT_TEXT("He told me a joke.");
}

///////////////////////////////////////////////////////////////////////////////////////////////////

int main() {
	testing::InitGoogleTest();
	return RUN_ALL_TESTS();
}
