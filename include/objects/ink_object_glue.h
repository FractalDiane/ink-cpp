#pragma once

#include "objects/ink_object.h"

#include <string>

class InkObjectGlue : public InkObject {
private:
	

public:
	virtual ObjectId get_id() const override { return ObjectId::Glue; }
	virtual std::string to_string() const override { return "Glue"; }

	virtual void execute(InkStoryData* const story_data, InkStoryState& story_state, InkStoryEvalResult& eval_result) override;
};
