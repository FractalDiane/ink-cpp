#include "objects/ink_object_sequence.h"

#include "ink_utils.h"

InkObjectSequence::~InkObjectSequence() {
	for (const auto& item : items) {
		for (InkObject* object : item) {
			delete object;
		}
	}
}

void InkObjectSequence::execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	std::size_t index = 0;
	switch (sequence_type) {
		case InkSequenceType::Shuffle:
		case InkSequenceType::ShuffleOnce:
		case InkSequenceType::ShuffleStop: {
			if (available_shuffle_indices.empty()) {
				if (sequence_type == InkSequenceType::ShuffleOnce) {
					return;
				} else if (sequence_type == InkSequenceType::ShuffleStop) {
					index = items.size() - 1;
				}
			} else {
				std::size_t rand_index = static_cast<std::size_t>(randi_range(0, available_shuffle_indices.size() - 1, story_state.rng));
				index = available_shuffle_indices[rand_index];
				
				if (sequence_type != InkSequenceType::Shuffle) {
					available_shuffle_indices.erase(std::remove(available_shuffle_indices.begin(), available_shuffle_indices.end(), index));
				}
			}
		} break;

		default: {
			index = current_index;
		} break;
	}

	if (sequence_type != InkSequenceType::OnceOnly || index < items.size()) {
		for (InkObject* object : items[index]) {
			object->execute(story_state, eval_result);
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
