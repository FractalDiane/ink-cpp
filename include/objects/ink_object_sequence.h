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
	bool multiline;
	std::vector<Knot> items;
	std::size_t current_index;
	std::vector<std::size_t> available_shuffle_indices;

	void fill_shuffle_indices();

public:
	InkObjectSequence(InkSequenceType type, bool multiline, const std::vector<Knot>& items)
		: sequence_type{type}, multiline{multiline}, items{items}, current_index{0}, available_shuffle_indices{} {
			fill_shuffle_indices();
		}

	virtual ~InkObjectSequence() override;

	virtual ObjectId get_id() const override { return ObjectId::Sequence; }

	virtual void execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) override;

	virtual bool contributes_content_to_knot() const override;

	virtual ByteVec to_bytes() const override;
	virtual InkObject* populate_from_bytes(const ByteVec& bytes, std::size_t& index) override;

	virtual bool stop_before_this(const InkStoryState& story_state) const override { return multiline; }
};
