#include "objects/ink_object_list.h"

#include "ink_utils.h"
#include "types/ink_list.h"

ByteVec Serializer<InkObjectList::ListDefinitionEntry>::operator()(const InkObjectList::ListDefinitionEntry& entry) {
	Serializer<std::string> sstring;
	Serializer<std::int64_t> s64;

	ByteVec result = sstring(entry.name);
	ByteVec result2 = s64(entry.value);
	result.insert(result.end(), result2.begin(), result2.end());
	result.push_back(static_cast<std::uint8_t>(entry.is_included_by_default));
	return result;
}

InkObjectList::ListDefinitionEntry Deserializer<InkObjectList::ListDefinitionEntry>::operator()(const ByteVec& bytes, std::size_t& index) {
	Deserializer<std::string> dsstring;
	Deserializer<std::int64_t> ds64;
	Deserializer<std::uint8_t> ds8;

	InkObjectList::ListDefinitionEntry result;
	result.name = dsstring(bytes, index);
	result.value = ds64(bytes, index);
	result.is_included_by_default = static_cast<bool>(ds8(bytes, index));

	return result;
}

ByteVec InkObjectList::to_bytes() const {
	Serializer<std::string> sstring;
	VectorSerializer<ListDefinitionEntry> vsentry;
	
	ByteVec result = sstring(name);
	ByteVec result2 = vsentry(entries);

	result.insert(result.end(), result2.begin(), result2.end());

	return result;
}

InkObject* InkObjectList::populate_from_bytes(const ByteVec& bytes, std::size_t& index) {
	Deserializer<std::string> dsstring;
	VectorDeserializer<ListDefinitionEntry> vdsentry;

	name = dsstring(bytes, index);
	entries = vdsentry(bytes, index);

	return this;
}

void InkObjectList::execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	InkList new_list_var;
	std::vector<InkListDefinitionEntry> list_entries;
	list_entries.reserve(entries.size());
	for (const ListDefinitionEntry& entry : entries) {
		list_entries.emplace_back(entry.name, entry.value);
		new_list_var.add_item(InkListItem(entry.name, entry.value, story_state.variable_info.current_list_definition_uuid));
	}

	story_state.variable_info.add_list_definition(list_entries);
	story_state.variable_info.variables[name] = new_list_var;
}
