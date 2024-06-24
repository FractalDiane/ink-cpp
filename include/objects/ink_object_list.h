#pragma once

#include "objects/ink_object.h"

#include <string>
#include <vector>

class InkObjectList : public InkObject {
public:
	struct ListDefinitionEntry {
		std::string name;
		std::int64_t value = 0;
		bool is_included_by_default = false;
	};

private:
	std::string name;
	std::vector<ListDefinitionEntry> entries;

public:
	InkObjectList(const std::string& name, const std::vector<ListDefinitionEntry>& entries) : name(name), entries(entries) {}

	virtual ObjectId get_id() const override { return ObjectId::List; }

	virtual ByteVec to_bytes() const override;
	virtual InkObject* populate_from_bytes(const ByteVec& bytes, std::size_t& index) override;

	virtual void execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) override;
};

template <>
struct Serializer<InkObjectList::ListDefinitionEntry> {
	ByteVec operator()(const InkObjectList::ListDefinitionEntry& entry);
};

template <>
struct Deserializer<InkObjectList::ListDefinitionEntry> {
	InkObjectList::ListDefinitionEntry operator()(const ByteVec& bytes, std::size_t& index);
};
