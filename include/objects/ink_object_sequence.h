#pragma once

#include "objects/ink_object.h"

#include <string>

enum class InkSequenceType {
	Sequence,
	Cycle,
	OnceOnly,
	Shuffle,
	ShuffleOnce,
	ShuffleStop,
};

class InkObjectSequence : public InkObject {
private:
	InkSequenceType sequence_type;
	std::vector<std::vector<InkObject*>> items;
	std::size_t current_index;

public:
	InkObjectSequence(InkSequenceType type, const std::vector<std::vector<InkObject*>>& items)
		: sequence_type{type}, items{items}, current_index{0} {}

	virtual ~InkObjectSequence() override;

	virtual ObjectId get_id() const override { return ObjectId::Sequence; }

	virtual void execute(InkStoryData* const story_data, InkStoryState& story_state, InkStoryEvalResult& eval_result) override;
};
