#pragma once

#include "objects/ink_object.h"

#include <string>
#include <vector>

class InkObjectList : public InkObject {
private:
	std::string name;
	std::vector<InkListDefinition::Entry> entries;

public:
	InkObjectList(const std::string& name, const std::vector<InkListDefinition::Entry>& entries) : name(name), entries(entries) {}

	virtual ObjectId get_id() const override { return ObjectId::List; }

	virtual ByteVec to_bytes() const override;
	virtual InkObject* populate_from_bytes(const ByteVec& bytes, std::size_t& index) override;

	virtual void execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) override;
};
