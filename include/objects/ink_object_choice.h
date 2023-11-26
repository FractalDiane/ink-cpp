#pragma once

#include "objects/ink_object.h"

#include <string>

struct InkChoiceEntry {
	std::vector<InkObject*> text;
	std::vector<InkObject*> result;
	bool sticky;
	std::vector<std::string> conditions;
	bool fallback;
};

class InkObjectChoice : public InkObject {
private:
	std::vector<InkChoiceEntry> choices;
	bool has_gather = false;

public:
	InkObjectChoice(const std::vector<InkChoiceEntry>& choices, bool has_gather) : choices{choices}, has_gather{has_gather} {}

	virtual ObjectId get_id() const override { return ObjectId::Choice; }
};
