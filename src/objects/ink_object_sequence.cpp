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
	std::size_t index = sequence_type == InkSequenceType::Shuffle ? randi_range(0, items.size() - 1) : current_index;
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
	

	/*switch (sequence_type) {
		case InkSequenceType::Sequence: {
			for (InkObject* object : items[current_index]) {
				object->execute(story_state, eval_result);
			}

			if (current_index < items.size() - 1) {
				++current_index;
			}
		} break;

		case InkSequenceType::Cycle: {
			for (InkObject* object : items[current_index]) {
				object->execute(story_state, eval_result);
			}

			current_index = (current_index + 1) % items.size();
		} break;

		case InkSequenceType::OnceOnly: {
			if (current_index < items.size()) {
				for (InkObject* object : items[current_index]) {
					object->execute(story_state, eval_result);
				}

				++current_index;
			}
		} break;

		case InkSequenceType::Shuffle: {
			for (InkObject* object : items[randi_range(0, items.size() - 1)]) {
				object->execute(story_state, eval_result);
			}
		} break;
	}*/
}
