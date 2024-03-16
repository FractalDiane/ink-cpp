#pragma once

#include "objects/ink_object.h"

class InkObjectChoiceTextMix : public InkObject {
private:
	bool end = false;

public:
	InkObjectChoiceTextMix(bool end) : end{end} {}

	virtual std::string to_string() const override;

	virtual ObjectId get_id() const override { return ObjectId::ChoiceTextMix; }

	virtual void execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) override;

	virtual ByteVec to_bytes() const override;
	virtual InkObject* populate_from_bytes(const ByteVec& bytes, std::size_t& index) override;

	bool is_end() const { return end; }
};
