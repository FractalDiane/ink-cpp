#include <gtest/gtest.h>

#include "ink_compiler.h"
#include "runtime/ink_story.h"
#include "ink_utils.h"

#include "expression_parser/expression_parser.h"

#include <utility>
#include <cstdlib>

#define FIXTURE(name) class name : public testing::Test {\
protected:\
	InkCompiler compiler;\
}

#define STORY(path) InkStory story = compiler.compile_file(INKCPP_WORKING_DIR "/tests/" path)
//#define STORY(path) compiler.compile_file_to_file(INKCPP_WORKING_DIR "/tests/" path, std::string(INKCPP_WORKING_DIR "/tests/" path) + "b");
//	InkStory story{std::string(INKCPP_WORKING_DIR "/tests/" path) + "b"};

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

template <typename TT, typename DT>
bool token_matches(ExpressionParserV2::Token& token, DT data) {
	if (auto* token_cast = dynamic_cast<TT*>(token)) {
		return token_cast->data == data;
	}

	return false;
}

#define EXPECT_TOKENS(tokens, ...) {\
	std::vector<ExpressionParser::TokenType> types = {__VA_ARGS__};\
	EXPECT_EQ(types.size(), tokens.size());\
	for (unsigned int i = 0; i < tokens.size(); ++i) {\
		EXPECT_EQ(tokens[i].type, types[i]);\
	}\
}

////////////////////////////////////////////////////////////////////////////////////////////////////
FIXTURE(NonStoryFunctionTests);
FIXTURE(ExpressionParserTests);

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
FIXTURE(NestedFlowTests);
FIXTURE(TrackingWeaveTests);
FIXTURE(GlobalVariableTests);
FIXTURE(LogicTests);
FIXTURE(ConditionalBlockTests);
FIXTURE(TemporaryVariableTests);
FIXTURE(FunctionTests);
FIXTURE(ConstantTests);
FIXTURE(TunnelTests);
FIXTURE(ThreadTests);
FIXTURE(ListTests);

FIXTURE(MiscellaneousTests);

FIXTURE(InkProof);

#pragma region NonStoryFunctionTests
/*TEST_F(NonStoryFunctionTests, DeinkifyExpression) {
	EXPECT_EQ(deinkify_expression("true and true"), "true && true");
	EXPECT_EQ(deinkify_expression("true or false"), "true || false");
	EXPECT_EQ(deinkify_expression("not true"), "! true");

	EXPECT_EQ(deinkify_expression("-> my_story"), "\"my_story\"");

	EXPECT_EQ(deinkify_expression("var++"), "var = var + 1");
	EXPECT_EQ(deinkify_expression("++var"), "var = var + 1");
	EXPECT_EQ(deinkify_expression("var--"), "var = var - 1");
	EXPECT_EQ(deinkify_expression("--var"), "var = var - 1");
}*/

TEST_F(NonStoryFunctionTests, InkListFunctions) {
	InkListDefinitionMap definition_map;
	definition_map.add_list_definition("colors", {{"red", 1}, {"orange", 2}, {"yellow", 3}});
	definition_map.add_list_definition("days", {{"sunday", 1}, {"monday", 2}, {"tuesday", 3}, {"wednesday", 4}, {"thursday", 5}, {"friday", 6}, {"saturday", 7}});

	InkList best_days{{"friday", "saturday", "sunday"}, definition_map};

	InkList weekends{{"saturday", "sunday"}, definition_map};
	EXPECT_TRUE(best_days.contains(weekends));

	EXPECT_EQ(best_days + weekends, best_days);
	EXPECT_EQ(best_days - weekends, InkList({"friday"}, definition_map));
	EXPECT_EQ(best_days ^ weekends, weekends);

	InkList all{{"sunday", "monday", "tuesday", "wednesday", "thursday", "friday", "saturday"}, definition_map};
	InkList all2 = best_days.all_possible_items();
	EXPECT_EQ(all, all2);

	InkList inverse{{"monday", "tuesday", "wednesday", "thursday"}, definition_map};
	InkList inverse2 = best_days.inverted();
	EXPECT_EQ(inverse, inverse2);

	InkList combined_list{{"red", "yellow", "tuesday"}, definition_map};
	EXPECT_FALSE(combined_list.contains(weekends));

	EXPECT_EQ(combined_list + weekends, InkList({"red", "yellow", "tuesday", "saturday", "sunday"}, definition_map));
	EXPECT_EQ(combined_list - weekends, combined_list);
	EXPECT_EQ(combined_list ^ weekends, InkList(definition_map));
	
	InkList all_c{{"sunday", "monday", "tuesday", "wednesday", "thursday", "friday", "saturday", "red", "orange", "yellow"}, definition_map};
	InkList all2_c = combined_list.all_possible_items();
	EXPECT_EQ(all_c, all2_c);
	
	InkList inverse_c{{"orange", "monday", "wednesday", "thursday", "friday", "saturday", "sunday"}, definition_map};
	InkList inverse2_c = combined_list.inverted();
	EXPECT_EQ(inverse_c, inverse2_c);
}
#pragma endregion

#pragma region ExpressionParserTests
TEST_F(ExpressionParserTests, BasicTokenization) {
	namespace ExpressionParser = ExpressionParserV2;
	using ExpressionParser::Token;
	using ExpressionParser::TokenType;
	std::string exp = "test = 5 + 7";

	ExpressionParser::StoryVariableInfo blank_variable_info;

	std::vector<ExpressionParser::Token> result = ExpressionParser::tokenize_expression(exp, blank_variable_info);

	EXPECT_TOKENS(result,
		ExpressionParser::TokenType::Variable,
		ExpressionParser::TokenType::Operator,
		ExpressionParser::TokenType::LiteralNumberInt,
		ExpressionParser::TokenType::Operator,
		ExpressionParser::TokenType::LiteralNumberInt,
	);

	std::vector<ExpressionParser::Token> result_postfix = ExpressionParser::shunt(result);
	EXPECT_EQ(result.size(), result_postfix.size());
	
	ExpressionParser::ExecuteResult result_token = ExpressionParser::execute_expression_tokens(result_postfix, blank_variable_info);
	EXPECT_FALSE(result_token.has_value());
	EXPECT_EQ(static_cast<std::int64_t>(blank_variable_info.variables["test"]), 12);
}

TEST_F(ExpressionParserTests, ExpressionEvaluation) {
	namespace ExpressionParser = ExpressionParserV2;
	using ExpressionParser::Variant;
	using ExpressionParser::execute_expression;

	ExpressionParser::StoryVariableInfo blank_variable_info;

	Variant t1 = execute_expression("5 + 7", blank_variable_info).value();
	EXPECT_EQ(static_cast<std::int64_t>(t1), 12);

	Variant t2 = execute_expression("5 + 7 * 52 - 8", blank_variable_info).value();
	EXPECT_EQ(static_cast<std::int64_t>(t2), 361);

	Variant t3 = execute_expression("5.0 * 4.2", blank_variable_info).value();
	EXPECT_EQ(static_cast<double>(t3), 21.0);

	Variant t4 = execute_expression("5.0 - 4.2 - 3.7 / 2.5", blank_variable_info).value();
	EXPECT_TRUE(std::abs(static_cast<double>(t4) - -0.68) < 0.0001);

	Variant t5 = execute_expression("5 == 2", blank_variable_info).value();
	EXPECT_EQ(static_cast<bool>(t5), false);

	Variant t6 = execute_expression("3 != 6", blank_variable_info).value();
	EXPECT_EQ(static_cast<bool>(t6), true);

	Variant t7 = execute_expression("7 < 12", blank_variable_info).value();
	EXPECT_EQ(static_cast<bool>(t7), true);

	Variant t8 = execute_expression(R"("hello" + " " + "there")", blank_variable_info).value();
	EXPECT_EQ(static_cast<std::string>(t8), "hello there");

	/*Variant t9 = execute_expression("++5", blank_variable_info).value();
	EXPECT_EQ(static_cast<std::int64_t>(t9), 6);

	Variant t10 = execute_expression("5++", blank_variable_info).value();
	EXPECT_EQ(static_cast<std::int64_t>(t10), 5);*/

	Variant t11 = execute_expression(R"("hello" ? "llo")", blank_variable_info).value();
	EXPECT_EQ(static_cast<bool>(t11), true);

	Variant t12 = execute_expression(R"("hello" ? "blah")", blank_variable_info).value();
	EXPECT_EQ(static_cast<bool>(t12), false);

	Variant t13 = execute_expression("5 * (3 + 4)", blank_variable_info).value();
	EXPECT_EQ(static_cast<std::int64_t>(t13), 35);

	Variant t14 = execute_expression("POW(3, 2)", blank_variable_info).value();
	EXPECT_EQ(static_cast<double>(t14), 9);

	Variant t15 = execute_expression("-> my_knot", blank_variable_info).value();
	EXPECT_EQ(static_cast<std::string>(t15), "my_knot");

	Variant t16 = execute_expression("POW(FLOOR(3.5), FLOOR(2.9)", blank_variable_info).value();
	EXPECT_EQ(static_cast<double>(t16), 9);

	Variant t17 = execute_expression("(5 * 5) - (3 * 3) + 3", blank_variable_info).value();
	EXPECT_EQ(static_cast<std::int64_t>(t17), 19);

	ExpressionParser::StoryVariableInfo vars;
	vars.variables = {{"x", 5}, {"y", 3}, {"c", 3}};
	execute_expression("x = (x * x) - (y * y) + c", vars);
	EXPECT_EQ(static_cast<std::int64_t>(vars.variables["x"]), 19);

	ExpressionParser::StoryVariableInfo vars2;
	vars2.variables = {{"test", 6}};
	Variant t19 = execute_expression("POW(test, 2)", vars2).value();
	EXPECT_EQ(static_cast<double>(t19), 36);

	ExpressionParser::StoryVariableInfo vars3;
	vars3.variables = {{"x", 5}};
	execute_expression("x++", vars3);
	EXPECT_EQ(static_cast<std::int64_t>(vars3.variables["x"]), 6);

	ExpressionParser::StoryVariableInfo vars4;
	vars4.variables = {{"visited_snakes", true}, {"dream_about_snakes", false}};
	Variant t20 = execute_expression("visited_snakes && not dream_about_snakes", vars4).value();
	EXPECT_EQ(static_cast<bool>(t20), true);
}
#pragma endregion

#pragma region ContentTests
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
#pragma endregion

#pragma region ChoiceTests
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
#pragma endregion

#pragma region KnotTests
TEST_F(KnotTests, Knots) {
	STORY("3_knots/3a_knot.ink");
	EXPECT_TEXT("Hello world");
}
#pragma endregion

#pragma region DivertTests
TEST_F(DivertTests, Diverts) {
	STORY("4_diverts/4a_divert.ink");
	EXPECT_TEXT("We hurried home to Savile Row as fast as we could.");
}

TEST_F(DivertTests, DivertsWithGlue) {
	STORY("4_diverts/4b_glue.ink");
	EXPECT_TEXT("We hurried home to Savile Row as fast as we could.");
}
#pragma endregion

#pragma region BranchingTests
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
#pragma endregion

#pragma region StitchTests
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
#pragma endregion

#pragma region VaryingChoiceTests
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
#pragma endregion

#pragma region VariableTextTests
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
#pragma endregion

#pragma region GameQueriesTests
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
#pragma endregion

#pragma region GatherTests
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
#pragma endregion

#pragma region NestedFlowTests
TEST_F(NestedFlowTests, NestedChoices) {
	for (int i = 0; i < 2; ++i) {
		STORY("11_nested_flow/11a_nested_choice.ink");
		EXPECT_TEXT(R"("Well, Poirot? Murder or suicide?")");

		EXPECT_CHOICES(R"("Murder!")", R"("Suicide!")");
		story.choose_choice_index(i);
		if (i == 0) {
			EXPECT_TEXT(R"("Murder!")", R"("And who did it?")");
			EXPECT_CHOICES(R"("Detective-Inspector Japp!")", R"("Captain Hastings!")", R"("Myself!")");
			story.choose_choice_index(1);
			EXPECT_TEXT(R"("Captain Hastings!")");
		} else {
			EXPECT_TEXT(R"("Suicide!")");
		}

		EXPECT_TEXT("Mrs. Christie lowered her manuscript a moment. The rest of the writing group sat, open-mouthed.");
	}
}

TEST_F(NestedFlowTests, NestedChoicesInBoth) {
	for (int i = 0; i < 2; ++i) {
		STORY("11_nested_flow/11b_nested_choices_in_both.ink");
		EXPECT_TEXT(R"("Well, Poirot? Murder or suicide?")");

		EXPECT_CHOICES(R"("Murder!")", R"("Suicide!")");
		story.choose_choice_index(i);
		if (i == 0) {
			EXPECT_TEXT(R"("Murder!")", R"("And who did it?")");
			EXPECT_CHOICES(R"("Detective-Inspector Japp!")", R"("Captain Hastings!")", R"("Myself!")");
			story.choose_choice_index(1);
			EXPECT_TEXT(R"("Captain Hastings!")");
		} else {
			EXPECT_TEXT(R"("Suicide!")", R"("Really, Poirot? Are you quite sure?")");
			EXPECT_CHOICES(R"("Quite sure.")", R"("It is perfectly obvious.")")
			story.choose_choice_index(0);
			EXPECT_TEXT(R"("Quite sure.")");
		}

		EXPECT_TEXT("Mrs. Christie lowered her manuscript a moment. The rest of the writing group sat, open-mouthed.");
	}
}

TEST_F(NestedFlowTests, NestedGathers) {
	for (int i = 0; i < 2; ++i) {
		STORY("11_nested_flow/11c_nested_gathers.ink");
		EXPECT_TEXT(R"("Well, Poirot? Murder or suicide?")");

		EXPECT_CHOICES(R"("Murder!")", R"("Suicide!")");
		story.choose_choice_index(i);
		if (i == 0) {
			EXPECT_TEXT(R"("Murder!")", R"("And who did it?")");
			EXPECT_CHOICES(R"("Detective-Inspector Japp!")", R"("Captain Hastings!")", R"("Myself!")");
			story.choose_choice_index(1);
			EXPECT_TEXT(R"("Captain Hastings!")", R"("You must be joking!")");
			EXPECT_CHOICES(R"("Mon ami, I am deadly serious.")", R"("If only...")");
			story.choose_choice_index(0);
			EXPECT_TEXT(R"("Mon ami, I am deadly serious.")");
		} else {
			EXPECT_TEXT(R"("Suicide!")", R"("Really, Poirot? Are you quite sure?")");
			EXPECT_CHOICES(R"("Quite sure.")", R"("It is perfectly obvious.")")
			story.choose_choice_index(0);
			EXPECT_TEXT(R"("Quite sure.")");
		}

		EXPECT_TEXT("Mrs. Christie lowered her manuscript a moment. The rest of the writing group sat, open-mouthed.");
	}
}

TEST_F(NestedFlowTests, DeepNesting) {
	std::vector<std::string> texts = {
		R"("It was a dark and stormy night...")",
		R"("...and the crew were restless...")",
		R"("... and they said to their Captain...")",
		R"("...Tell us a tale Captain!")",
	};

	for (int i = 0; i < 2; ++i) {
		STORY("11_nested_flow/11d_deep_nesting.ink");
		EXPECT_TEXT(R"("Tell us a tale, Captain!")");
		EXPECT_CHOICES(R"("Very well, you sea-dogs. Here's a tale...")", R"("No, it's past your bed-time.")");
		story.choose_choice_index(i);
		if (i == 0) {
			EXPECT_TEXT(R"("Very well, you sea-dogs. Here's a tale...")");
			for (const std::string& text : texts) {
				EXPECT_CHOICES(text);
				story.choose_choice_index(0);
				EXPECT_TEXT(text);
			}
		} else {
			EXPECT_TEXT(R"("No, it's past your bed-time.")");
		}

		EXPECT_TEXT("To a man, the crew began to yawn.");
	}
}

TEST_F(NestedFlowTests, ComplexNesting) {
	for (int i = 0; i < 3; ++i) {
		STORY("11_nested_flow/11e_complex_nesting.ink");
		EXPECT_TEXT("I looked at Monsieur Fogg");
		EXPECT_CHOICES("... and I could contain myself no longer.", "... but I said nothing");
		if (i == 0) {
			story.choose_choice_index(1);
			EXPECT_TEXT("... but I said nothing and we passed the day in silence.");
		} else {
			story.choose_choice_index(0);
			EXPECT_TEXT("... and I could contain myself no longer.", "'What is the purpose of our journey, Monsieur?'", "'A wager,' he replied.");
			EXPECT_CHOICES("'A wager!'", "'Ah.'");
			story.choose_choice_index(0);
			EXPECT_TEXT("'A wager!' I returned.", "He nodded.");
			EXPECT_CHOICES("'But surely that is foolishness!'", "'A most serious matter then!'");
			story.choose_choice_index(1);
			EXPECT_TEXT("'A most serious matter then!'", "He nodded again.");
			EXPECT_CHOICES("'But can we win?'", "'A modest wager, I trust?'", "I asked nothing further of him then.");
			if (i == 1) {
				story.choose_choice_index(1);
				EXPECT_TEXT("'A modest wager, I trust?'", "'Twenty thousand pounds,' he replied, quite flatly.", "After that, we passed the day in silence.");
			} else {
				story.choose_choice_index(2);
				EXPECT_TEXT("I asked nothing further of him then, and after a final, polite cough, he offered nothing more to me. After that, we passed the day in silence.");
			}
		}
	}
}
#pragma endregion

#pragma region TrackingWeaveTests
TEST_F(TrackingWeaveTests, ChoiceLabels) {
	for (int i = 0; i < 2; ++i) {
		STORY("12_tracking_weave/12a_choice_labels.ink");
		EXPECT_TEXT("The guard frowns at you.");
		EXPECT_CHOICES("Greet him", "'Get out of my way.'");

		if (i == 0) {
			story.choose_choice_index(0);
			EXPECT_TEXT("'Greetings.'", "'Hmm,' replies the guard.");
			EXPECT_CHOICES("'Having a nice day?'", "'Hmm?'");
			story.choose_choice_index(0);
			EXPECT_TEXT("'Having a nice day?'", "'Mff,' the guard replies, and then offers you a paper bag. 'Toffee?'");
		} else {
			story.choose_choice_index(1);
			EXPECT_TEXT("'Get out of my way,' you tell the guard.", "'Hmm,' replies the guard.");
			EXPECT_CHOICES("'Hmm?'", "Shove him aside");
			story.choose_choice_index(1);
			EXPECT_TEXT("You shove him sharply. He stares in reply, and draws his sword!", "You appear to be in trouble.");
		}
	}
}

TEST_F(TrackingWeaveTests, WeaveScope) {
	STORY("12_tracking_weave/12b_weave_scope.ink");
	EXPECT_TEXT("");
	EXPECT_CHOICES("one");
	story.choose_choice_index(0);
	EXPECT_TEXT("Some content.");
	EXPECT_CHOICES("two", "end");
	story.choose_choice_index(0);
	EXPECT_TEXT("two");
	EXPECT_CHOICES("Option");
	story.choose_choice_index(0);
	EXPECT_TEXT("Option", "");
}

TEST_F(TrackingWeaveTests, ChoiceOptionLabels) {
	for (int i = 0; i < 2; ++i) {
		STORY("12_tracking_weave/12a_choice_labels.ink");
		EXPECT_TEXT("The guard frowns at you.");
		EXPECT_CHOICES("Greet him", "'Get out of my way.'");
		story.choose_choice_index(1);
		EXPECT_TEXT("'Get out of my way,' you tell the guard.", "'Hmm,' replies the guard.");
		EXPECT_CHOICES("'Hmm?'", "Shove him aside");
		story.choose_choice_index(1);
		EXPECT_TEXT("You shove him sharply. He stares in reply, and draws his sword!", "You appear to be in trouble.");

		EXPECT_CHOICES("Throw rock at guard", "Throw sand at guard");
		story.choose_choice_index(i);
		EXPECT_TEXT(i == 0 ? "You hurl a rock at the guard." : "You hurl a handful of sand at the guard.",
		"The guard thrusts his sword through your chest. That went about as well as you expected.");
	}
}

TEST_F(TrackingWeaveTests, WeaveLoops) {
	for (int i = 0; i < 2; ++i) {
		STORY("12_tracking_weave/12d_weave_loops.ink");
		EXPECT_TEXT("");
		EXPECT_CHOICES("'Can I get a uniform from somewhere?'", "'Tell me about the security system.'", "'Are there dogs?'");
		story.choose_choice_index(0);
		EXPECT_TEXT("'Can I get a uniform from somewhere?' you ask the cheerful guard.",
		"'Sure. In the locker.' He grins. 'Don't think it'll fit you, though.'");

		EXPECT_CHOICES("'Tell me about the security system.'", "'Are there dogs?'", "Enough talking");
		if (i == 0) {
			story.choose_choice_index(2);
			EXPECT_TEXT("You thank the guard, and move away.");
		} else {
			story.choose_choice_index(0);
			EXPECT_TEXT("'Tell me about the security system.'", "'It's ancient,' the guard assures you. 'Old as coal.'");
			EXPECT_CHOICES("'Are there dogs?'", "Enough talking");
			story.choose_choice_index(0);
			EXPECT_TEXT("'Are there dogs?'", "'Hundreds,' the guard answers, with a toothy grin. 'Hungry devils, too.'",
			"He scratches his head.", "'Well, can't stand around talking all day,' he declares.", "You thank the guard, and move away.");
		}
	}
}
#pragma endregion

#pragma region GlobalVariableTests
TEST_F(GlobalVariableTests, VariableChecks) {
	std::vector<std::tuple<int, bool>> var_values = {
		{0, false},
		{1, false},
		{0, true},
		{1, true},
	};

	std::vector<std::vector<std::string>> choices_expected = {
		{"'But, Monsieur, why are we travelling?'"},
		{"'But, Monsieur, why are we travelling?'"},
		{"I contemplated our strange adventure"},
		{"I contemplated our strange adventure"},
	};

	for (int i = 0; i < 4; ++i) {
		STORY("13_global_variables/13a_variable_checks.ink");

		int mood = std::get<0>(var_values[i]);
		bool knows_about_wager = std::get<1>(var_values[i]);

		story.set_variable("mood", mood);
		story.set_variable("knows_about_wager", knows_about_wager);

		EXPECT_TEXT(std::format("The train jolted and rattled. {}.",
								mood > 0 ? "I was feeling positive enough, however, and did not mind the odd bump"
								: "It was more than I could bear"));

		EXPECT_EQ(story.get_current_choices(), choices_expected[i]);
	}
}

TEST_F(GlobalVariableTests, DivertVariables) {
	STORY("13_global_variables/13b_divert_variables.ink");

	EXPECT_TEXT("Your Kingdom is in dire straits.", "Give up now, or keep trying to save your Kingdom?");
	EXPECT_CHOICES("Keep trying!", "Give up");
	story.choose_choice_index(0);
	EXPECT_TEXT("It's not working. Your Kingdom is still in dire straits.", "Give up now, or keep trying to save your Kingdom?");
	EXPECT_CHOICES("Keep trying!", "Give up");
	story.choose_choice_index(0);
	EXPECT_TEXT("It's not working. Your Kingdom is still in dire straits.", "Give up now, or keep trying to save your Kingdom?");
	EXPECT_CHOICES("Keep trying!", "Give up");
	story.choose_choice_index(1);
	EXPECT_TEXT("Everybody in your Kingdom died.");
}

TEST_F(GlobalVariableTests, PrintingVariables) {
	STORY("13_global_variables/13c_printing_variables.ink");
	EXPECT_TEXT("My name is Jean Passepartout, but my friends call me Jackie. I'm 23 years old.");
}

/*TEST_F(GlobalVariableTests, EvalLogicInString) {
	// TODO: string logic
	STORY("13_global_variables/13d_string_logic.ink");

	std::string text = story.continue_story();
	EXPECT_FALSE(text.contains("{~red|blue|green|yellow}"));

	std::string end = text.substr(52);
	std::size_t split_index = end.find(" and ");

	std::string left = end.substr(0, split_index);
	std::string right = end.substr(split_index + 5);
	right.pop_back();
	EXPECT_EQ(left, right);
}*/
#pragma endregion

#pragma region LogicTests
TEST_F(LogicTests, BasicLogic) {
	STORY("14_logic/14a_basic_logic.ink");
	EXPECT_TEXT("knows_about_wager = true", "x = 27", "y = 108", "p1 = 9", "p2 = 4");
}

TEST_F(LogicTests, StringComparisons) {
	STORY("14_logic/14b_string_comparisons.ink");
	EXPECT_TEXT("true", "true", "true");
}
#pragma endregion

#pragma region ConditionalBlockTests
TEST_F(ConditionalBlockTests, BasicIf) {
	for (int i = 0; i < 2; ++i) {
		STORY("15_conditional_blocks/15a_basic_if.ink");
		story.set_variable("x", i == 0 ? 0 : 7);
		story.set_variable("y", 0);

		EXPECT_TEXT(std::format("y = {}", i == 0 ? 0 : 6));
	}
}

TEST_F(ConditionalBlockTests, IfElse) {
	for (int i = 0; i < 2; ++i) {
		STORY("15_conditional_blocks/15b_if_else.ink");
		story.set_variable("x", i == 0 ? 0 : 7);
		story.set_variable("y", 0);

		EXPECT_TEXT(std::format("y = {}", i == 0 ? 1 : 6));
	}
}

TEST_F(ConditionalBlockTests, IfElseAlt) {
	for (int i = 0; i < 2; ++i) {
		STORY("15_conditional_blocks/15c_if_else_alt.ink");
		story.set_variable("x", i == 0 ? 0 : 7);
		story.set_variable("y", 0);

		EXPECT_TEXT(std::format("y = {}", i == 0 ? 1 : 6));
	}
}

TEST_F(ConditionalBlockTests, IfElifElse) {
	for (int i = 0; i < 3; ++i) {
		STORY("15_conditional_blocks/15d_if_elif_else.ink");
		story.set_variable("x", i == 0 ? 0 : i == 1 ? 7 : -2);
		story.set_variable("y", 3);

		EXPECT_TEXT(std::format("y = {}", i == 0 ? 0 : i == 1 ? 6 : -1));
	}
}

TEST_F(ConditionalBlockTests, SwitchStatement) {
	std::vector<std::string> expected_results = {"zero", "one", "two", "lots"};
	for (int i = 0; i < 4; ++i) {
		STORY("15_conditional_blocks/15e_switch_statement.ink");
		story.set_variable("x", i);

		EXPECT_TEXT(expected_results[i]);
	}
}

TEST_F(ConditionalBlockTests, ReadCountCondition) {
	std::vector<std::tuple<int, bool, bool>> inputs = {
		{0, false, false},
		{0, true, false},
		{0, false, true},	
	};

	std::vector<std::string> expected_result = {
		"You had a dream about marmalade. You prefer strawberry jam, but it's not bad.",
		"You had a dream about snakes. Thousands of them. Oh dear.",
		"You had a dream about Polish beer. Unfortunately, you don't drink.",
	};

	std::vector<int> expected_fear = {0, 1, -1};

	for (int i = 0; i < 3; ++i) {
		STORY("15_conditional_blocks/15f_read_count_condition.ink");
		story.set_variable("fear", std::get<0>(inputs[i]));
		story.set_variable("visited_snakes", std::get<1>(inputs[i]));
		story.set_variable("visited_poland",std::get<2>(inputs[i]));

		EXPECT_TEXT(expected_result[i]);
		EXPECT_EQ(static_cast<std::int64_t>(story.get_variable("fear").value()), expected_fear[i]);
	}
}

TEST_F(ConditionalBlockTests, ConditionalWithLogic) {
	for (int i = 0; i < 2; ++i) {
		STORY("15_conditional_blocks/15g_conditional_with_content.ink");
		story.set_variable("know_about_wager", i == 1);

		EXPECT_TEXT(std::format("I stared at Monsieur Fogg. {}", i == 1 ? "\"But surely you are not serious?\" I demanded." : "\"But there must be a reason for this trip,\" I observed."));
		EXPECT_TEXT("He said nothing in reply, merely considering his newspaper with as much thoroughness as entomologist considering his latest pinned addition.");
	}
}

TEST_F(ConditionalBlockTests, ConditionalWithOptions) {
	for (int i = 0; i < 2; ++i) {
		STORY("15_conditional_blocks/15h_conditional_with_options.ink");
		story.set_variable("door_open", i == 1);

		EXPECT_TEXT("Monsieur Fogg and I stared at each other for several long moments.");
		EXPECT_EQ(story.get_current_choices(), 
			(i == 0 ? std::vector<std::string>{"I asked permission to leave", "I stood and went to open the door"}
			: std::vector<std::string>{"I strode out of the compartment"})
		);

		story.choose_choice_index(0);
		if (i == 0) {
			EXPECT_TEXT("I asked permission to leave and Monsieur Fogg looked surprised. I opened the door and left.");
		} else {
			EXPECT_TEXT("I strode out of the compartment and I fancied I heard my master quietly tutting to himself. I went outside.");
		}
	}
}

TEST_F(ConditionalBlockTests, MultilineAlternatives) {
	STORY("15_conditional_blocks/15i_multiline_alternatives.ink");
	EXPECT_TEXT("I entered the casino.");
	std::string next = story.continue_story();
	EXPECT_TRUE(next.starts_with("At the table, I drew a card."));
	EXPECT_TEXT("I held my breath.", "Would my luck hold?");

	story.choose_choice_index(0);
	EXPECT_TEXT("I entered the casino again.");
	std::string next2 = story.continue_story();
	EXPECT_TRUE(next2.starts_with("At the table, I drew a card."));
	EXPECT_TEXT("I waited impatiently.", "Could I win the hand?");

	story.choose_choice_index(0);
	EXPECT_TEXT("Once more, I went inside.");
	std::string next3 = story.continue_story();
	EXPECT_TRUE(next3.starts_with("At the table, I drew a card."));
	EXPECT_TEXT("I paused.", "");
}

TEST_F(ConditionalBlockTests, ModifiedShuffles) {
	STORY("15_conditional_blocks/15j_modified_shuffles.ink");

	std::string one_1 = story.continue_story();
	EXPECT_TRUE(one_1 == "The sun was hot." || one_1 == "It was a hot day.");
	std::string two_1 = story.continue_story();
	EXPECT_TRUE(two_1 == "A silver BMW roars past." || two_1 == "A bright yellow Mustang takes the turn.");

	story.choose_choice_index(0);
	std::string one_2 = story.continue_story();
	EXPECT_TRUE((one_2 == "The sun was hot." || one_2 == "It was a hot day.") && one_2 != one_1);
	std::string two_2 = story.continue_story();
	EXPECT_TRUE((two_2 == "A silver BMW roars past." || two_2 == "A bright yellow Mustang takes the turn.") && two_2 != two_1);

	story.choose_choice_index(0);
	EXPECT_TEXT("There are like, cars, here.");
	story.choose_choice_index(0);
	EXPECT_TEXT("There are like, cars, here.");
	story.choose_choice_index(0);
	EXPECT_TEXT("There are like, cars, here.");
}
#pragma endregion

#pragma region TemporaryVariableTests
TEST_F(TemporaryVariableTests, TemporaryVariables) {
	for (int i = 0; i < 2; ++i) {
		STORY("16_temporary_variables/16a_temporary_variables.ink");
		story.set_variable("blanket", true);
		story.set_variable("ear_muffs", i == 1);
		story.set_variable("gloves", i == 1);
		EXPECT_TEXT(i == 0 ? "That night I was colder than I have ever been." : "Despite the snow, I felt incorrigibly snug.");
	}
}

TEST_F(TemporaryVariableTests, KnotParameters) {
	std::vector<std::string> accused = {"Hastings", "Claudia", "myself"};
	std::vector<std::string> remarks = {"Sewing machines", "Supercalifragilisticexpialidocious", "Cow"};

	for (int i = 0; i < 3; ++i) {
		STORY("16_temporary_variables/16b_knot_parameters.ink");
		EXPECT_TEXT("");
		story.choose_choice_index(i);
		EXPECT_TEXT(std::format("\"I accuse {}!\" Poirot declared. \"{}!\"", accused[i], remarks[i]), std::format("\"Really?\" Japp replied. \"{}?\"", i == 2 ? "You did it" : accused[i]), "\"And why not?\" Poirot shot back.");
	}
}

TEST_F(TemporaryVariableTests, KnotRecursion) {
	STORY("16_temporary_variables/16c_knot_recursion.ink");
	EXPECT_TEXT("\"The result is 5050!\" you announce.", "Gauss stares at you in horror.");
}

TEST_F(TemporaryVariableTests, DivertsAsArguments) {
	STORY("16_temporary_variables/16d_diverts_as_arguments.ink");
	EXPECT_TEXT("You lie down and close your eyes.", "You sleep perchance to dream etc. etc.", "You get back to your feet, ready to continue your journey.");
}
#pragma endregion

#pragma region FunctionTests
TEST_F(FunctionTests, FunctionsInLogic) {
	STORY("17_functions/17a_functions_in_logic.ink");
	std::string text = story.continue_story();
	EXPECT_TRUE(text.starts_with("x = 3."));
}

TEST_F(FunctionTests, FunctionsInInterpolation) {
	STORY("17_functions/17b_functions_in_interpolation.ink");
	EXPECT_TEXT("");
	EXPECT_CHOICES("Yes.");
	story.choose_choice_index(0);
	EXPECT_TEXT("Yes.");
}

TEST_F(FunctionTests, FunctionSideEffects) {
	STORY("17_functions/17c_function_side_effects.ink");
	EXPECT_TEXT("You have 10 HP.",);
	EXPECT_TEXT("A stray pebble flies in and hits you on the head. It deals no damage, but you are so startled that you fall over and land on your face.");
	EXPECT_TEXT("You took 2 damage.")
	EXPECT_TEXT("You now have 8 HP.")
}

TEST_F(FunctionTests, InlineFunctionCall) {
	STORY("17_functions/17d_inline_function_call.ink");
	story.set_variable("health", 50);
	EXPECT_TEXT("Monsieur Fogg was looking somewhat flagging.");
}

TEST_F(FunctionTests, InlineFunctionCallWithoutReturn) {
	STORY("17_functions/17d2_inline_function_call_without_return.ink");
	story.set_variable("health", 50);
	EXPECT_TEXT("Monsieur Fogg was looking somewhat flagging.");
}

TEST_F(FunctionTests, NestedFunctionCalls) {
	STORY("17_functions/17e_nested_function_calls.ink");
	EXPECT_TEXT("The maximum of 2^5 and 3^3 is 32.");
}

TEST_F(FunctionTests, PrintNumFunction) {
	STORY("17_functions/17f_print_num.ink");
	EXPECT_TEXT(
		"I pulled out fifteen coins from my pocket and slowly counted them.",
		R"("Oh, never mind," the trader replied. "I'll take half." And she took seven and pushed the rest back over to me.)",
	);
}

TEST_F(FunctionTests, FunctionParamsByRef) {
	for (int i = 0; i < 2; ++i) {
		STORY("17_functions/17g_function_params_by_ref.ink");
		EXPECT_TEXT("You have 10 HP.", "Monsieur Fogg has 14 HP.");
		EXPECT_CHOICES("I ate a biscuit", "I gave a biscuit to Monsieur Fogg");
		story.choose_choice_index(i);
		EXPECT_TEXT(std::format("{} Then we continued on our way.",
									i == 0
									? "I ate a biscuit and felt refreshed."
									: "I gave a biscuit to Monsieur Fogg and he wolfed it down most undecorously."));

		EXPECT_TEXT(
			std::format("You have {} HP.", i == 0 ? 12 : 10),
			std::format("Monsieur Fogg has {} HP.", i == 0 ? 14 : 15),
		);
	}
}

TEST_F(FunctionTests, ComplexFunctions) {
	STORY("17_functions/17h_complex_functions.ink");
	EXPECT_TEXT("hello");
	EXPECT_TEXT("there");
	EXPECT_TEXT("x = 3.8000000");
	EXPECT_TEXT("y = 2");
	EXPECT_TEXT("hello");
	EXPECT_TEXT("therehello");
	EXPECT_TEXT("there4");
	EXPECT_TEXT("hello");
	EXPECT_TEXT("there");
	EXPECT_TEXT("this");
	EXPECT_TEXT("is");
	EXPECT_TEXT("a");
	EXPECT_TEXT("testhi");
	EXPECT_CHOICES("Yes.", "No.");
	story.choose_choice_index(0);
	EXPECT_TEXT("Yes.");
}
#pragma endregion

#pragma region ConstantTests
TEST_F(ConstantTests, StringConstants) {
	for (int i = 0; i < 2; ++i) {
		STORY("18_constants/18a_string_constants.ink");
		story.set_variable("found_japps_bloodied_glove", i == 1);
		EXPECT_TEXT(std::format("Current Suspect: {}", i == 0 ? "Hastings" : "Poirot"));
	}
}

TEST_F(ConstantTests, NumberConstants) {
	STORY("18_constants/18b_number_constants.ink");
	EXPECT_TEXT(
		"The circumference of my pie divided by its diameter is approximately 3.1400000",
		"I paid 10 pounds for said pie, by the way.",
	);
}

TEST_F(ConstantTests, LogicWithConstants) {
	STORY("18_constants/18c_constants_logic.ink");
	EXPECT_TEXT(
		"The secret agent moves forward.",
		"The secret agent moves forward.",
		"The secret agent grabs the suitcase!",
	);

	EXPECT_EQ(static_cast<std::int64_t>(story.get_variable("suitcase_location").value()), -1);
}
#pragma endregion

#pragma region TunnelTests
TEST_F(TunnelTests, BasicTunnels) {
	STORY("19_tunnels/19a_basic_tunnels.ink");
	EXPECT_TEXT("The train rattled and rolled. When suddenly...");
	EXPECT_CHOICES("\"Monsieur!\"");
	story.choose_choice_index(0);
	EXPECT_TEXT(
		"\"Monsieur!\" I declared with sudden horror. \"I have just realised. We have crossed the international date line!\"",
		"Monsieur Fogg barely lifted an eyebrow. \"I have adjusted for it.\"",
	);

	EXPECT_CHOICES("I mopped the sweat from my brow", "I nodded, becalmed", "I cursed, under my breath");
	story.choose_choice_index(0);
	EXPECT_TEXT("I mopped the sweat from my brow. A relief!", "Our journey continued. On the return trip, all was well, when suddenly...!");

	EXPECT_CHOICES("\"Monsieur!\"");
	story.choose_choice_index(0);
	EXPECT_TEXT(
		"\"Monsieur!\" I declared with sudden horror. \"I have just realised. We have crossed the international date line!\"",
		"Monsieur Fogg barely lifted an eyebrow. \"I have adjusted for it.\"",
	);

	EXPECT_CHOICES("I nodded, becalmed", "I cursed, under my breath");
	story.choose_choice_index(1);
	EXPECT_TEXT("I cursed, under my breath. Once again, I had been belittled!", "Our journey continued. Again.");
}

TEST_F(TunnelTests, NestedTunnels) {
	STORY("19_tunnels/19b_nested_tunnels.ink");
	EXPECT_TEXT("The dark grass is soft under your feet.");
	EXPECT_CHOICES("Sleep");
	story.choose_choice_index(0);

	EXPECT_TEXT(
		"You lie down and try to close your eyes.",
		"A goblin attacks you!",
		"Irritated, you yell \"GO AWAY!\" at the top of your lungs.",
		"The goblin dejectedly slinks away, defeated.",
		"Then it is time to sleep.",
	);

	EXPECT_TEXT("You have a dream about snakes. Is this a shameless callback to test case 15F? Absolutely.");
	EXPECT_TEXT("You wake as the sun rises.");
	EXPECT_CHOICES("Eat something", "Make a move");
	story.choose_choice_index(0);

	EXPECT_TEXT(
		"You pull out a submarine sandwich from your coat pocket and take a bite.",
		"How exactly did it fit in there? You don't know nor care.",
		"It is time to move on.",
	);
}

TEST_F(TunnelTests, TunnelReturnTargets) {
	for (int i = 0; i < 2; ++i) {
		STORY("19_tunnels/19c_tunnel_return_targets.ink");
		story.set_variable("stamina", 5 + (1 - i));

		EXPECT_TEXT("You're walking down the street, but then the street turns into a cliff and you fall off it.");
		EXPECT_TEXT("YOU TOOK 5 DAMAGE");
		if (i == 0) {
			EXPECT_TEXT("You're still alive! You pick yourself up and walk on.");
		} else {
			EXPECT_TEXT("Suddenly, there is a white light all around you. Fingers lift an eyepiece from your forehead. 'You lost, buddy. Out of the chair.'");
		}
	}
}

TEST_F(TunnelTests, AdvancedTunnelStructure) {
	STORY("19_tunnels/19d_advanced_tunnel_structure.ink");
	EXPECT_TEXT("You approach Jim confidently. He doesn't look like he's in the best mood.", "\"What do you need, cadet?\"");
	EXPECT_CHOICES("Ask about the warp nacelles", "Ask about the shield generators", "Stop talking");
	story.choose_choice_index(0);

	EXPECT_TEXT("\"Don't worry about the warp nacelles. They're fine.\"");
	EXPECT_CHOICES("Ask about the shield generators", "Stop talking");
	story.choose_choice_index(0);

	EXPECT_TEXT(
		"\"What's with all these questions?\" Jim demands, suddenly.",
		"\"Uh...\" you reply. \"Just wanted to make sure everything's good, is all.\"",
		"\"You're far too anxious, cadet,\" he admonishes you. \"Learn to trust people to do their jobs.\"",
	);

	EXPECT_CHOICES("Stop talking");
	story.choose_choice_index(0);

	EXPECT_TEXT("Jim leaves to tend to more important matters.");
}

TEST_F(TunnelTests, InlineTunnels) {
	STORY("19_tunnels/19e_inline_tunnels.ink");
	EXPECT_TEXT("C++ uses manual memory management, so there is no automatic freeing of allocated memory. Cow. Many people understandably find this annoying.");
}
#pragma endregion

#pragma region ThreadTests
TEST_F(ThreadTests, BasicThreads) {
	STORY("20_threads/20a_basic_threads.ink");
	EXPECT_TEXT(
		"I had a headache; threading is hard to get your head around.",
		"It was a tense moment for Monty and me.",
		"We continued to walk down the dusty road.",
	);

	EXPECT_CHOICES("\"What did you have for lunch today?\"", "\"Nice weather, we're having,\"", "Continue walking");
	story.choose_choice_index(1);

	EXPECT_TEXT("\"Nice weather, we're having,\" I said.", "\"I've seen better,\" he replied.", "Before long, we arrived at his house.");
}

TEST_F(ThreadTests, NestedThreads) {
	STORY("20_threads/20b_nested_threads.ink");
	EXPECT_TEXT(
		"I had a headache; threading is hard to get your head around.",
		"It was a tense moment for Monty and me.",
		"We continued to walk down the dusty road.",
		"HEY THERE",
	);

	EXPECT_CHOICES("\"What did you have for lunch today?\"", "\"Nice weather, we're having,\"", "Choice", "Continue walking");

	story.choose_choice_index(2);
	EXPECT_TEXT("Before long, we arrived at his house.");
}

TEST_F(ThreadTests, ComplexThreads) {
	STORY("20_threads/20c_complex_threads.ink");
	EXPECT_TEXT("");
	EXPECT_CHOICES("Ask the General about the bloodied knife", "Drawers", "Wardrobe", "Go to Office");

	story.choose_choice_index(0);
	EXPECT_TEXT(R"("It's a bad business, I can tell you.")");
	EXPECT_CHOICES("Drawers", "Wardrobe", "Go to Office");
	story.choose_choice_index(1);
	EXPECT_TEXT("This wardrobe looks conspicuous.");
	EXPECT_CHOICES("Drawers", "Go to Office");
	story.choose_choice_index(1);
	EXPECT_TEXT("");

	EXPECT_CHOICES("Ask the Doctor about the bloodied knife", "Cabinets", "Computer", "Go to Hallway");
	story.choose_choice_index(0);
	EXPECT_TEXT(R"("There's nothing strange about blood, is there?")");
	EXPECT_CHOICES("Cabinets", "Computer", "Go to Hallway");
	story.choose_choice_index(0);
	EXPECT_TEXT("These cabinets look incriminating.");
	EXPECT_CHOICES("Computer", "Go to Hallway");
	story.choose_choice_index(1);
	EXPECT_TEXT("");

	EXPECT_CHOICES("Drawers", "Go to Office");
	story.choose_choice_index(0);
	EXPECT_TEXT("These drawers look suspicious.");
}

TEST_F(ThreadTests, ThreadDivertParameters) {
	STORY("20_threads/20d_thread_divert_parameters.ink");
	EXPECT_TEXT("The front step. The house smells. Of murder. And lavender.");
	EXPECT_CHOICES("Review my case notes", "Go through the front door", "Sniff the air");
	story.choose_choice_index(2);
	EXPECT_TEXT("I hate lavender. It makes me think of soap, and soap makes me think about my marriage.");
	EXPECT_CHOICES("Review my case notes", "Go through the front door");
	story.choose_choice_index(1);
	EXPECT_TEXT("I stepped inside the house.", "The hallway. Front door open to the street. Little bureau.");
	EXPECT_CHOICES("Review my case notes", "Go through the front door", "Open the bureau");
	story.choose_choice_index(2);
	EXPECT_TEXT("Keys. More keys. Even more keys. How many locks do these people need?");
	EXPECT_CHOICES("Review my case notes", "Go through the front door");
	story.choose_choice_index(1);
	EXPECT_TEXT("I stepped out into the cool sunshine.", "The front step. The house smells. Of murder. And lavender.");
	EXPECT_CHOICES("Review my case notes", "Go through the front door");
	story.choose_choice_index(0);
	EXPECT_TEXT("I flicked through the notes I'd made so far. Still not obvious suspects.");
	
	for (int i = 0; i < 5; ++i) {
		EXPECT_CHOICES("Go through the front door");
		story.choose_choice_index(0);
		EXPECT_TEXT("I stepped inside the house.", "The hallway. Front door open to the street. Little bureau.");
		EXPECT_CHOICES("Go through the front door");
		story.choose_choice_index(0);
		EXPECT_TEXT("I stepped out into the cool sunshine.", "The front step. The house smells. Of murder. And lavender.");
	}

	EXPECT_CHOICES("Go through the front door");
	story.choose_choice_index(0);
	EXPECT_TEXT("I stepped inside the house.", "The hallway. Front door open to the street. Little bureau.");
	EXPECT_CHOICES("Review my case notes", "Go through the front door");
	story.choose_choice_index(0);
	EXPECT_TEXT("Once again, I flicked through the notes I'd made so far. Still not obvious suspects.");
}
#pragma endregion

#pragma region ListTests
TEST_F(ListTests, BasicLists) {
	STORY("21_lists/21a_basic_lists.ink");
	EXPECT_TEXT("The kettle is cold");
	EXPECT_CHOICES("Turn on kettle", "Touch the kettle");

	story.choose_choice_index(1);
	EXPECT_TEXT("The kettle is cool to the touch.");
	EXPECT_CHOICES("Turn on kettle", "Touch the kettle");
	story.choose_choice_index(0);
	EXPECT_TEXT("The kettle begins to bubble and boil.");
	EXPECT_CHOICES("Touch the kettle");
	story.choose_choice_index(0);
	EXPECT_TEXT("The outside of the kettle is very warm!");
}

TEST_F(ListTests, ListDefaultValue) {
	STORY("21_lists/21b_list_default_value.ink");
	EXPECT_TEXT("The kettle is already boiling. Edgy, huh?");
}

TEST_F(ListTests, ReusingLists) {
	STORY("21_lists/21c_reusing_lists.ink");
	EXPECT_TEXT("Today is today, also known as Monday. Tomorrow is tomorrow, also known as Tuesday.");
}

TEST_F(ListTests, ListNamespaces) {
	STORY("21_lists/21d_list_namespaces.ink");
	EXPECT_TEXT("I'm blue, da ba dee da ba die");
}

TEST_F(ListTests, ListValues) {
	STORY("21_lists/21e_list_values.ink");
	EXPECT_TEXT("The lecturer is quiet.");
	EXPECT_TEXT("There is some quiet murmuring in the hall.");
	EXPECT_TEXT("The lecturer raises their voice. They are now medium.");
	EXPECT_TEXT("The murmuring gets louder. It's now medium.");
	EXPECT_TEXT("The lecturer raises their voice. They are now loud.");
	EXPECT_TEXT("The murmuring gets louder. It's now loud.");
	EXPECT_TEXT("The lecturer raises their voice. They are now deafening.");
	EXPECT_TEXT("The murmuring gets louder. It's now deafening.");
	EXPECT_TEXT("The lecturer yells at the top of their lungs!");
	EXPECT_TEXT("They give themself a sore throat and leave the room.");
}

TEST_F(ListTests, ExplicitListValues) {
	STORY("21_lists/21f_list_explicit_values.ink");
	EXPECT_TEXT("One is the loneliest number, two can be as bad as one");
}

TEST_F(ListTests, ListValueNumbers) {
	STORY("21_lists/21g_list_value_numbers.ink");
	EXPECT_TEXT("The lecturer has 2 notches still available to him.");
}

TEST_F(ListTests, ListNumbersToValues) {
	STORY("21_lists/21h_list_numbers_to_values.ink");
	EXPECT_TEXT("You have two points");
}

TEST_F(ListTests, MultipleListValues) {
	STORY("21_lists/21i_list_multiple_values.ink");
	EXPECT_TEXT("Adams, Cartwright are currently in surgery; please direct complaints to Bernard or Eamonn.");
}

TEST_F(ListTests, MultipleListValuesAssignment) {
	STORY("21_lists/21j_list_multiple_values_assignment.ink");
	EXPECT_TEXT("Adams, Cartwright, Denver are currently in surgery; please direct complaints to Bernard or Eamonn.");
}

TEST_F(ListTests, ListAssignEmpty) {
	STORY("21_lists/21k_list_assign_empty.ink");
	EXPECT_TEXT("The doctors currently in surgery are:");
}

TEST_F(ListTests, ListAddValuesInPlace) {
	STORY("21_lists/21l_list_add_values_in_place.ink");
	EXPECT_TEXT("The doctors currently in surgery are: Denver, Eamonn");
	EXPECT_TEXT("The doctors currently in surgery are: Adams, Bernard, Denver, Eamonn");
	EXPECT_TEXT("The doctors currently in surgery are: Adams, Bernard, Cartwright, Denver, Eamonn");
	EXPECT_TEXT("The doctors currently in surgery are: Adams, Cartwright, Denver, Eamonn");
	EXPECT_TEXT("The doctors currently in surgery are: Cartwright, Denver");
}

TEST_F(ListTests, ListQueries) {
	STORY("21_lists/21m_list_queries.ink");
	EXPECT_TEXT("2");
	EXPECT_TEXT("Adams");
	EXPECT_TEXT("Cartwright");
	std::string random = story.continue_story();
	EXPECT_TRUE(random == "Adams" || random == "Cartwright");
}

TEST_F(ListTests, ListContentTests) {
	STORY("21_lists/21n_list_contents_tests.ink");
	EXPECT_TEXT("The surgery is open today.");
	EXPECT_TEXT("Dr Adams and Dr Cartwright are having a loud argument in one corner.");
	EXPECT_TEXT("At least Adams and Bernard aren't arguing.");
	EXPECT_TEXT("Dr Eamonn is polishing his glasses.");
	EXPECT_TEXT("Dr Denver appears to be taking the day off.");
	EXPECT_TEXT("false");
	EXPECT_TEXT("I smiled politely.");
	EXPECT_TEXT("'Monsieur, really!' I cried. 'I know Englishmen are strange, but this is *incredible*!'");
}

TEST_F(ListTests, NiceListPrintFunction) {
	STORY("21_lists/21o_nice_list_print.ink");
	EXPECT_TEXT("My favourite dinosaurs are stegosaurs, anklyosaurus and pleiosaur.");
}

TEST_F(ListTests, ListQueries2) {
	STORY("21_lists/21p_list_queries_2.ink");
	EXPECT_TEXT("The doctors that work here are: Adams, Bernard, Cartwright, Denver, Eamonn");
	EXPECT_TEXT("There are 5 doctors that work here.");
	EXPECT_TEXT("Adams");
	EXPECT_TEXT("eleven, thirteen, seventeen, nineteen");
}

TEST_F(ListTests, RefreshListType) {
	STORY("21_lists/21q_refresh_list_type.ink");
	EXPECT_TEXT("first_value, second_value, third_value");
}
#pragma endregion

#pragma region Miscellaneous Tests
TEST_F(MiscellaneousTests, UnicodeSupport) {
	STORY("miscellaneous/unicode_support.ink");
	EXPECT_TEXT("Well, it's £1 for a five-minute argument, but it'll be £8 for a course of ten.");
}

static int observer_test = 0;

TEST_F(MiscellaneousTests, VariableObservation) {
	STORY("miscellaneous/variable_observation.ink");
	story.observe_variable("test", [](const std::string& var_name, const ExpressionParserV2::Variant& new_value) { ++observer_test; });
	EXPECT_EQ(observer_test, 0);
	
	story.continue_story();
	story.continue_story();
	EXPECT_EQ(static_cast<std::int64_t>(observer_test), 1);
}
#pragma endregion

#pragma region InkProof
TEST_F(InkProof, MinimalStory) {
	STORY("ink-proof/1_minimal_story.ink");
	EXPECT_TEXT("Hello, world!");
}

TEST_F(InkProof, FoggComfortsPassepartout) {
	STORY("ink-proof/2_fogg_passepartout.ink");
	EXPECT_TEXT(R"("What's that?" my master asked.)");
	EXPECT_CHOICES(
		R"("I am somewhat tired.")",
		R"("Nothing, Monsieur!")",
		R"("I said, this journey is appalling.")",
	);

	story.choose_choice_index(2);

	EXPECT_TEXT(
		R"("I said, this journey is appalling and I want no more of it.")",
		R"("Ah," he replied, not unkindly. "I see you are feeling frustrated. Tomorrow, things will improve.")",
	);
}

TEST_F(InkProof, TunnelToDeath) {
	STORY("ink-proof/3_tunnel_to_death.ink");
	EXPECT_TEXT("Should you cross the river?");
	EXPECT_CHOICES("Yes", "No");

	story.choose_choice_index(1);
	EXPECT_TEXT(
		"You follow the path along the river for some time and finally encounter a huge man with a wooden stick.",
		"As you start talking to him, he beats you with his weapon.",
	);

	EXPECT_CHOICES("Fight back", "Flee");
	story.choose_choice_index(1);

	EXPECT_TEXT("You desperately run for your life and never look back.");
}

TEST_F(InkProof, PrintNumAsEnglish) {
	STORY("ink-proof/4_print_num_as_english.ink");
	EXPECT_TEXT("You have fifty-eight coins.");
}

TEST_F(InkProof, ConstVariable) {
	STORY("ink-proof/5_const_variable.ink");
	EXPECT_TEXT("5");
}

TEST_F(InkProof, MultipleConstantRefs) {
	STORY("ink-proof/6_multiple_constant_refs.ink");
	EXPECT_TEXT("success");
}

TEST_F(InkProof, SetNonexistentVariable) {
	STORY("ink-proof/7_set_nonexistent_variable.ink");
	EXPECT_TEXT("Hello world.");
}

TEST_F(InkProof, TempGlobalConflict) {
	STORY("ink-proof/8_temp_global_conflict.ink");
	EXPECT_TEXT("0");
}

TEST_F(InkProof, TempInOptions) {
	STORY("ink-proof/9_temp_in_options.ink");
	EXPECT_TEXT("");
	EXPECT_CHOICES("1");
	story.choose_choice_index(0);
	EXPECT_TEXT("1", "End of choice", "this another");
}

TEST_F(InkProof, TempNotFound) {
	STORY("ink-proof/10_temp_not_found.ink");
	EXPECT_TEXT("hello");
}

TEST_F(InkProof, TempAtGlobalScope) {
	STORY("ink-proof/11_temp_at_global_scope.ink");
	EXPECT_TEXT("54");
}

TEST_F(InkProof, VarDeclareInConditional) {
	STORY("ink-proof/12_var_declare_in_conditional.ink");
	EXPECT_TEXT("5");
}

TEST_F(InkProof, VariableDivertTarget) {
	STORY("ink-proof/13_variable_divert_target.ink");
	EXPECT_TEXT("Here.");
}

TEST_F(InkProof, VarSwapRecurse) {
	STORY("ink-proof/14_var_swap_recurse.ink");
	EXPECT_TEXT("1 2");
}

TEST_F(InkProof, VariableTunnel) {
	STORY("ink-proof/15_variable_tunnel.ink");
	EXPECT_TEXT("STUFF");
}

TEST_F(InkProof, Empty) {
	STORY("ink-proof/16_empty.ink");
	EXPECT_TEXT("");
}

TEST_F(InkProof, End) {
	STORY("ink-proof/17_end.ink");
	EXPECT_TEXT("hello", "");
}

TEST_F(InkProof, End2) {
	STORY("ink-proof/18_end2.ink");
	EXPECT_TEXT("hello", "");
}

TEST_F(InkProof, EndOfContent) {
	STORY("ink-proof/19_end_of_content.ink");
	EXPECT_TEXT("");
}

TEST_F(InkProof, EscapeCharacter) {
	STORY("ink-proof/20_escape_character.ink");
	EXPECT_TEXT("this is a '|' character");
}

TEST_F(InkProof, IdentifiersStartingWithNumbers) {
	STORY("ink-proof/21_identifiers_start_with_numbers.ink");
	EXPECT_TEXT("512x2 = 1024", "512x2p2 = 1026");
}

/*TEST_F(InkProof, QuoteSignificance) {
	STORY("ink-proof/22_quote_significance.ink");
	EXPECT_TEXT("My name is \"Joe\"");
	EXPECT_TEXT("My name is \"Joe\"");
}*/

TEST_F(InkProof, Whitespace) {
	STORY("ink-proof/23_whitespace.ink");
	EXPECT_TEXT("Hello!", "World.");
}

TEST_F(InkProof, Includes) {
	STORY("ink-proof/24_includes.ink");
	EXPECT_TEXT("This is include 1.", "This is include 2.", "This is the main file.");
}

TEST_F(InkProof, NestedIncludes) {
	STORY("ink-proof/25_nested_includes.ink");
	EXPECT_TEXT("The value of a variable in test file 2 is 5.", "This is the main file", "The value when accessed from knot_in_2 is 5.");
}

TEST_F(InkProof, FloorCeilingCast) {
	STORY("ink-proof/26_floor_ceiling_cast.ink");
	EXPECT_TEXT(
		"1",
		"1",
		"2",
		"0.6666667",
		"0",
		"1",
	);
}

TEST_F(InkProof, ReadCountAcrossCallStack) {
	STORY("ink-proof/27_read_count_across_callstack.ink");
	EXPECT_TEXT(
		"1) Seen first 1 times.",
		"In second.",
		"2) Seen first 1 times.",
	);
}

TEST_F(InkProof, ReadCountAcrossThreads) {
	STORY("ink-proof/28_read_count_across_threads.ink");
	EXPECT_TEXT("1", "1");
}

TEST_F(InkProof, ReadCountDotSeparatedPath) {
	STORY("ink-proof/29_read_count_dot_separated_path.ink");
	EXPECT_TEXT("hi", "hi", "hi", "3");
}

TEST_F(InkProof, NestedTurnsSince) {
	STORY("ink-proof/30_nested_turns_since.ink");
	EXPECT_TEXT("-1 = -1");
	EXPECT_CHOICES("stuff");
	story.choose_choice_index(0);
	EXPECT_TEXT("stuff", "0 = 0");

	EXPECT_CHOICES("more stuff");
	story.choose_choice_index(0);
	EXPECT_TEXT("more stuff", "1 = 1");
}
#pragma endregion
////////////////////////////////////////////////////////////////////////////////////////////////////

int main() {
	testing::InitGoogleTest();
	return RUN_ALL_TESTS();
}
