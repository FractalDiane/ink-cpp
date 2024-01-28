#pragma once

#include "objects/ink_object.h"

#include <string>

class InkObjectConditional : public InkObject {
private:
	//std::unordered_map<std::string, std::vector<InkObject*>> branches;
	
	bool is_switch;
	std::vector<std::pair<std::string, Knot>> branches;
	Knot branch_else;

	std::string switch_expression;

public:
	InkObjectConditional(const std::vector<std::pair<std::string, Knot>>& branches, const Knot& objects_else)
		: branches{branches}, branch_else{objects_else}, is_switch{false} {}

	InkObjectConditional(const std::string& switch_expression, const std::vector<std::pair<std::string, Knot>>& branches, const Knot& objects_else)
		: switch_expression{switch_expression}, branches{branches}, branch_else{objects_else}, is_switch{true} {}
 
	virtual ~InkObjectConditional() override;

	virtual ObjectId get_id() const override { return ObjectId::Conditional; }

	virtual void execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) override;
};
