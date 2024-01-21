#pragma once

#include "objects/ink_object.h"

#include <string>

class InkObjectConditional : public InkObject {
private:
	//std::unordered_map<std::string, std::vector<InkObject*>> branches;
	std::vector<std::pair<std::string, std::vector<InkObject*>>> branches;
	std::vector<InkObject*> branch_else;

public:
	InkObjectConditional(const std::vector<std::pair<std::string, std::vector<InkObject*>>>& branches, const std::vector<InkObject*>& objects_else)
		: branches{branches}, branch_else{objects_else} {}
 
	virtual ~InkObjectConditional() override;

	virtual ObjectId get_id() const override { return ObjectId::Conditional; }

	virtual void execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) override;
};
