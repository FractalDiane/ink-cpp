#pragma once

#include "objects/ink_object.h"

#include <string>

class InkObjectLineBreak : public InkObject {
public:
	virtual ObjectId get_id() const override { return ObjectId::LineBreak; }
	virtual std::string to_string() const override { return "Line break"; }

	virtual void execute(InkStoryData* const story_data, InkStoryState& story_state, InkStoryEvalResult& eval_result) override;
};
