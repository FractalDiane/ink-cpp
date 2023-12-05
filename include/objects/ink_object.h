#pragma once

#include <string>
#include <vector>

#include "serialization.h"
#include "runtime/ink_story_state.h"

enum class ObjectId {
	Text,
	Choice,
	LineBreak,
	Glue,
	Divert,
	Interpolation,
	Conditional,
	Sequence,
	ChoiceTextMix,
	Tag,
	GlobalVariable,
};

class InkObject {
public:
	virtual ~InkObject() = default;

	virtual std::string to_string() const;
	virtual std::vector<std::uint8_t> to_bytes() const;
	virtual InkObject* populate_from_bytes(const std::vector<std::uint8_t>& bytes, std::size_t& index);
	virtual ObjectId get_id() const = 0;
	virtual bool has_any_contents() const { return true; }

	virtual void execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) = 0;

	virtual bool will_choice_take_fallback(InkStoryState& story_state) { return false; }
	
	std::vector<std::uint8_t> get_serialized_bytes() const;
	//InkObject* populate_from_bytes() const;
};

template <>
struct Serializer<InkObject*> {
	std::vector<std::uint8_t> operator()(const InkObject* object) noexcept {
		return object->get_serialized_bytes();
	}
};
