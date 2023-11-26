#pragma once

#include "objects/ink_object.h"

#include <string>
#include <format>

class InkObjectTag : public InkObject {
private:
	std::string tag;

public:
	InkObjectTag(const std::string& tag_text) : tag{tag_text} {}

	virtual ObjectId get_id() const override { return ObjectId::Tag; }
	virtual std::string to_string() const override { return std::format("Tag ({})", tag); }
};
