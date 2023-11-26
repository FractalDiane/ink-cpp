#pragma once

#include "objects/ink_object.h"

#include <string>
#include <format>

class InkObjectDivert : public InkObject {
private:
	std::string target_knot;

public:
	InkObjectDivert() : target_knot{std::string()} {}
	InkObjectDivert(const std::string& target) : target_knot{target} {}

	virtual ObjectId get_id() const override { return ObjectId::Divert; }
	virtual std::string to_string() const override { return std::format("Divert ({})", target_knot); }
};
