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
	virtual std::vector<std::uint8_t> to_bytes() const override;
	virtual InkObject* populate_from_bytes(const std::vector<std::uint8_t>& bytes, std::size_t& index) override;
	virtual std::string to_string() const override { return std::format("Divert ({})", target_knot); }
	virtual bool has_any_contents() const override { return true; }
};
