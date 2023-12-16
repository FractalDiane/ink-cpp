#include <gtest/gtest.h>

#include "ink_compiler.h"
#include "runtime/ink_story.h"

#include "builtin-features.inc"

#include <utility>

#define FIXTURE(name) class name : public testing::Test {\
protected:\
	InkCompiler compiler;\
}

#define STORY(path) InkStory story = compiler.compile_file(INKCPP_WORKING_DIR "/tests/" path)

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
FIXTURE(GameQueriesTests);
FIXTURE(GatherTests);

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

TEST_F(VariableTextTests, OnceOnlyWithEmptyEntries) {
	STORY("8_variable_text/8e_onceonly_empty.ink");
	EXPECT_TEXT("I took a step forward.");
	story.choose_choice_index(0);
	EXPECT_TEXT("I took a step forward.");
	story.choose_choice_index(0);
	EXPECT_TEXT("I took a step forward.");
	story.choose_choice_index(0);
	EXPECT_TEXT("I took a step forward.");
	story.choose_choice_index(0);
	EXPECT_TEXT("I took a step forward. Then the lights went out.");
}

TEST_F(VariableTextTests, CycleNested) {
	STORY("8_variable_text/8f_cycle_nested.ink");
	EXPECT_TEXT("The Ratbear wastes no time and swipes at you.");
	story.choose_choice_index(0);
	EXPECT_TEXT("The Ratbear scratches into your leg.");
	story.choose_choice_index(0);
	EXPECT_TEXT("The Ratbear swipes at you.");
	story.choose_choice_index(0);
	EXPECT_TEXT("The Ratbear scratches into your arm.");
	story.choose_choice_index(0);
	EXPECT_TEXT("The Ratbear swipes at you.");
	story.choose_choice_index(0);
	EXPECT_TEXT("The Ratbear scratches into your cheek.");
	story.choose_choice_index(0);
	EXPECT_TEXT("The Ratbear swipes at you.");
	story.choose_choice_index(0);
	EXPECT_TEXT("The Ratbear scratches into your leg.");
	story.choose_choice_index(0);
	EXPECT_TEXT("The Ratbear swipes at you.");
}

TEST_F(VariableTextTests, AlternativeWithDivert) {
	STORY("8_variable_text/8g_alternative_divert.ink");
	EXPECT_TEXT("I waited.");
	story.choose_choice_index(0);
	EXPECT_TEXT("I waited some more.");
	story.choose_choice_index(0);
	EXPECT_TEXT("I snoozed.");
	story.choose_choice_index(0);
	EXPECT_TEXT("I woke up and waited more.");
	story.choose_choice_index(0);
	EXPECT_TEXT("I gave up and left. I walked away in frustration.");
}

TEST_F(VariableTextTests, AlternativeInChoice) {
	STORY("8_variable_text/8h_alternative_in_choice.ink");
	EXPECT_TEXT(R"("Hello, boy," Monsieur Fogg proclaimed.)");
	EXPECT_CHOICES(R"("Hello, Master!")", "Say nothing");

	story.choose_choice_index(0);
	EXPECT_TEXT(R"("Hello, Monsieur Fogg!" I declared.)", R"("Hello, boy," Monsieur Fogg proclaimed.)");
	EXPECT_CHOICES(R"("Hello, you!")", "Say nothing");
	story.choose_choice_index(0);
	EXPECT_TEXT(R"("Hello, brown-eyes!" I declared.)", R"("Hello, boy," Monsieur Fogg proclaimed.)");
	EXPECT_CHOICES(R"("Hello, Master!")", "Say nothing");
	story.choose_choice_index(0);
	EXPECT_TEXT(R"("Hello, Monsieur Fogg!" I declared.)", R"("Hello, boy," Monsieur Fogg proclaimed.)");
	EXPECT_CHOICES(R"("Hello, you!")", "Say nothing");
	story.choose_choice_index(0);
	EXPECT_TEXT(R"("Hello, brown-eyes!" I declared.)", R"("Hello, boy," Monsieur Fogg proclaimed.)");
	EXPECT_CHOICES(R"("Hello, Master!")", "Say nothing");
	story.choose_choice_index(0);
	EXPECT_TEXT(R"("Hello, Monsieur Fogg!" I declared.)", R"("Hello, boy," Monsieur Fogg proclaimed.)");
	EXPECT_CHOICES(R"("Hello, you!")", "Say nothing");
	story.choose_choice_index(1);
	EXPECT_TEXT("You remain silent.", R"("Hello, boy," Monsieur Fogg proclaimed.)");
}

TEST_F(VariableTextTests, AlternativeAtChoiceStart) {
	STORY("8_variable_text/8i_alternative_at_choice_start.ink");
	story.continue_story();
	EXPECT_CHOICES("They headed towards the Sandlands");
	story.choose_choice_index(0);
	EXPECT_TEXT("They set off for the desert", "");
	EXPECT_CHOICES("The party followed the old road South");
	story.choose_choice_index(0);
	EXPECT_TEXT("They headed towards the Sandlands", "");
	EXPECT_CHOICES("They set off for the desert",);
	story.choose_choice_index(0);
	EXPECT_TEXT("The party followed the old road South");
}

TEST_F(VariableTextTests, ConditionalText) {
	std::vector<std::pair<bool ,bool>> var_values = {{false, false}, {true, false}, {true, true}};
	std::vector<std::pair<std::string, std::string>> expected_text_outer = {
		{"There was a man.", "I missed him. Was he particularly evil?"},
		{"There was a man. He had a peculiar face.", "I saw him. Only for a moment. His real name was kept a secret."},
		{"There was a man. He had a peculiar face.", "I saw him. Only for a moment. His real name was Franz."},
	};

	for (std::size_t i = 0; i < expected_text_outer.size(); ++i) {
		STORY("8_variable_text/8k_conditional_text.ink");
		story.set_variable("met_blofeld", var_values[i].first);
		story.set_variable("learned_his_name", var_values[i].second);
		EXPECT_TEXT(expected_text_outer[i].first, expected_text_outer[i].second);
	}
}

TEST_F(GameQueriesTests, ChoiceCountFunction) {
	STORY("9_game_queries/9a_choice_count.ink");
	EXPECT_TEXT("");
	EXPECT_CHOICES("Option B", "Option C");
}

TEST_F(GameQueriesTests, TurnsFunction) {
	STORY("9_game_queries/9b_turns.ink");
	EXPECT_TEXT("0 turns");
	story.choose_choice_index(0);
	EXPECT_TEXT("1: 1", "1 turns");
	story.choose_choice_index(0);
	EXPECT_TEXT("1: 2");
}

TEST_F(GameQueriesTests, TurnsSinceFunction) {
	STORY("9_game_queries/9c_turns_since.ink");
	EXPECT_TEXT("It's been -1 turns now", "HELLO THERE", "It's been 0 turns now");
	story.choose_choice_index(0);
	EXPECT_TEXT("It's been 1 turn now");
}

TEST_F(GameQueriesTests, SeedRandomFunction) {
	STORY("9_game_queries/9d_seed_random.ink");
	std::string text1 = story.continue_story();
	std::string text2 = story.continue_story();
	EXPECT_EQ(text1, text2);
}

TEST_F(GatherTests, Gathers) {
	std::vector<std::vector<std::string>> expected_texts = {
		{R"("I am somewhat tired," I repeated.)", R"("Really," he responded. "How deleterious.")"},
		{R"("Nothing, Monsieur!" I replied.)", R"("Very good, then.")"},
		{R"("I said, this journey is appalling and I want no more of it.")", R"("Ah," he replied, not unkindly. "I see you are feeling frustrated. Tomorrow, things will improve.")"},
	};

	for (int i = 0; i < 3; ++i) {
		STORY("10_gathers/10a_gather.ink");
		EXPECT_TEXT(R"("What's that?" my master asked.)");

		EXPECT_CHOICES(R"("I am somewhat tired.")", R"("Nothing, Monsieur!")", R"("I said, this journey is appalling.")");
		story.choose_choice_index(i);
		EXPECT_TEXT(expected_texts[i][0], expected_texts[i][1], "With that Monsieur Fogg left the room.");
	}
}

TEST_F(GatherTests, MultipleGathers) {
	std::vector<std::string> expected_texts_1 = {
		"I checked the jewels were still in my pocket, and the feel of them brought a spring to my step. ",
		"I did not pause for breath but kept on running. ",
		"I cheered with joy. ",
	};

	std::vector<std::string> expected_texts_2 = {
		"I reached the road and looked about. And would you believe it?",
		"I should interrupt to say Mackie is normally very reliable. He's never once let me down. Or rather, never once, previously to that night.",
	};

	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 2; j++) {
			STORY("10_gathers/10b_multi_gather.ink");
			EXPECT_TEXT("I ran through the forest, the dogs snapping at my heels.");

			EXPECT_CHOICES("I checked the jewels", "I did not pause for breath", "I cheered with joy.");
			story.choose_choice_index(i);
			std::string text1 = expected_texts_1[i] + "The road could not be much further! Mackie would have the engine running, and then I'd be safe.";
			EXPECT_TEXT(text1);

			EXPECT_CHOICES("I reached the road and looked about", "I should interrupt to say Mackie is normally very reliable");
			story.choose_choice_index(j);
			EXPECT_TEXT(expected_texts_2[j], "The road was empty. Mackie was nowhere to be seen.");
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

int main() {
	cparse_startup();
	testing::InitGoogleTest();
	return RUN_ALL_TESTS();
}
