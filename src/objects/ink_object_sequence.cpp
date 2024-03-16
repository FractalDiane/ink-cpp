#include "objects/ink_object_sequence.h"

#include "ink_utils.h"

ByteVec InkObjectSequence::to_bytes() const {
	Serializer<std::uint8_t> s8;
	Serializer<std::uint16_t> s16;
	VectorSerializer<InkObject*> sobjects;

	ByteVec result = s8(static_cast<std::uint8_t>(sequence_type));
	ByteVec result2 = s8(static_cast<std::uint8_t>(multiline));
	
	std::uint16_t objects_size = static_cast<std::uint16_t>(items.size());
	ByteVec result3 = s16(objects_size);

	result.insert(result.end(), result2.begin(), result2.end());
	result.insert(result.end(), result3.begin(), result3.end());

	for (const auto& vec : items) {
		ByteVec result_objects = sobjects(vec);
		result.insert(result.end(), result_objects.begin(), result_objects.end());
	}

	return result;
}

InkObject* InkObjectSequence::populate_from_bytes(const ByteVec& bytes, std::size_t& index) {
	Deserializer<std::uint8_t> ds8;
	Deserializer<std::uint16_t> ds16;
	VectorDeserializer<InkObject*> dsobjects;

	sequence_type = static_cast<InkSequenceType>(ds8(bytes, index));
	multiline = static_cast<bool>(ds8(bytes, index));

	std::uint16_t objects_size = ds16(bytes, index);
	for (std::uint16_t i = 0; i < objects_size; ++i) {
		items.push_back(dsobjects(bytes, index));
	}

	return this;
}

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
