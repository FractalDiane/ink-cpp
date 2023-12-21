#pragma once

#include "objects/ink_object.h"

#include <string>

class InkObjectGlobalVariable : public InkObject {
private:
	std::string name;
	std::string value;

public:
	InkObjectGlobalVariable(const std::string& name, const std::string& value) : name{name}, value{value} {}

	virtual ObjectId get_id() const override { return ObjectId::GlobalVariable; }

	virtual void execute(InkStoryData* const story_data, InkStoryState& story_state, InkStoryEvalResult& eval_result) override;
};
