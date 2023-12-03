#pragma once

#include "objects/ink_object.h"

#include <string>

struct InkChoiceEntry {
	std::vector<InkObject*> text;
	Knot result;
	bool sticky = false;
	std::vector<std::string> conditions;
	bool fallback = false;
	bool immediately_continue_to_result = false;
};

class InkObjectChoice : public InkObject {
private:
	std::vector<InkChoiceEntry> choices;
	bool has_gather = false;

public:
	InkObjectChoice(const std::vector<InkChoiceEntry>& choices, bool has_gather) : choices{choices}, has_gather{has_gather} {}
	virtual ~InkObjectChoice() override;

	virtual std::string to_string() const override;
	virtual ObjectId get_id() const override { return ObjectId::Choice; }

	virtual void execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) override;
};
