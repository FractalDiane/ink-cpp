#pragma once

#include <string>
#include <vector>

#include "serialization.h"

enum ObjectId {
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
	virtual std::string to_string() const;
	virtual std::vector<std::uint8_t> to_bytes() const;
	virtual ObjectId get_id() const = 0;
	virtual bool has_any_contents() const { return false; }
	
	std::vector<std::uint8_t> get_serialized_bytes() const;
};
