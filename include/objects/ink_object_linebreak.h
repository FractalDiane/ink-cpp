#pragma once

#include "objects/ink_object.h"

#include <string>

class InkObjectLineBreak : public InkObject {
public:
	virtual ObjectId get_id() const override { return ObjectId::LineBreak; }
	virtual std::string to_string() const override { return "Line break"; }
};
