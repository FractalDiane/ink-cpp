#pragma once

#include "objects/ink_object.h"

#include <string>

class InkObjectGlobalVariable : public InkObject {
private:
	std::string name;
	bool is_constant = false;
	std::vector<struct ExpressionParser::Token*> value_shunted_tokens;

public:
	InkObjectGlobalVariable(const std::string& name, bool is_constant, const std::vector<struct ExpressionParser::Token*>& value_shunted_tokens) : name{name}, is_constant{is_constant}, value_shunted_tokens{value_shunted_tokens} {}

	virtual ~InkObjectGlobalVariable() override;

	virtual ObjectId get_id() const override { return ObjectId::GlobalVariable; }

	virtual void execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) override;
};
