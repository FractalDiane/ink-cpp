#pragma once

#include "objects/ink_object.h"

class InkObjectChoiceTextMix : public InkObject {
private:
	bool end = false;

public:
	InkObjectChoiceTextMix(bool end) : end{end} {}

	virtual std::string to_string() const override;

	virtual ObjectId get_id() const override { return ObjectId::ChoiceTextMix; }

	virtual void execute(InkStoryData* const story_data, InkStoryState& story_state, InkStoryEvalResult& eval_result) override;
};
