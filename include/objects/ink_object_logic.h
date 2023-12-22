#pragma once

#include "objects/ink_object.h"

#include <string>

class InkObjectLogic : public InkObject {
private:
	std::string contents;

public:
	InkObjectLogic(const std::string& contents) : contents{contents} {}

	virtual ObjectId get_id() const override { return ObjectId::Logic; }
	virtual std::string to_string() const override { return contents; }

	virtual void execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) override;
};
