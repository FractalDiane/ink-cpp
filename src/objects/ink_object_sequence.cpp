#include "objects/ink_object_sequence.h"

#include "ink_utils.h"

InkObjectSequence::~InkObjectSequence() {
	for (const auto& item : items) {
		for (InkObject* object : item) {
			delete object;
		}
	}
}

void InkObjectSequence::execute(InkStoryData* const story_data, InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	std::size_t index = sequence_type == InkSequenceType::Shuffle ? static_cast<std::size_t>(randi_range(0, items.size() - 1, story_state.rng)) : current_index;
	if (sequence_type != InkSequenceType::OnceOnly || index < items.size()) {
		for (InkObject* object : items[index]) {
			object->execute(story_data, story_state, eval_result);
		}

		switch (sequence_type) {
			case InkSequenceType::Sequence: {
				if (current_index < items.size() - 1) {
					++current_index;
				}
			} break;

			case InkSequenceType::Cycle: {
				current_index = (current_index + 1) % items.size();
			} break;

			case InkSequenceType::OnceOnly: {
				++current_index;
			} break;

			default: break;
		}
	}
}
