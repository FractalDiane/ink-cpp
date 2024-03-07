#pragma once

#include "objects/ink_object.h"

#include "expression_parser/expression_parser.h"

#include <string>

struct InkChoiceEntry {
	std::vector<InkObject*> text;
	Knot result;
	bool sticky = false;
	std::vector<std::vector<ExpressionParser::Token*>> conditions;
	bool fallback = false;
	bool immediately_continue_to_result = false;

	GatherPoint label;
};

class InkObjectChoice : public InkObject {
private:
	std::vector<InkChoiceEntry> choices;
	//bool has_gather = false;

public:
	//InkObjectChoice(const std::vector<InkChoiceEntry>& choices, bool has_gather) : choices{choices}, has_gather{has_gather} {}
	InkObjectChoice(const std::vector<InkChoiceEntry>& choices) : choices{choices} {}
	virtual ~InkObjectChoice() override;

	virtual std::string to_string() const override;
	virtual ObjectId get_id() const override { return ObjectId::Choice; }

	virtual void execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) override;

	virtual bool will_choice_take_fallback(InkStoryState& story_state) override;
	//virtual bool stop_before_this() const override { return true; }
};
