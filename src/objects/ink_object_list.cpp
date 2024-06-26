#include "objects/ink_object_list.h"

#include "ink_utils.h"
#include "types/ink_list.h"

ByteVec InkObjectList::to_bytes() const {
	Serializer<std::string> sstring;
	VectorSerializer<InkListDefinitionEntry> vsentry;
	
	ByteVec result = sstring(name);
	ByteVec result2 = vsentry(entries);

	result.insert(result.end(), result2.begin(), result2.end());

	return result;
}

InkObject* InkObjectList::populate_from_bytes(const ByteVec& bytes, std::size_t& index) {
	Deserializer<std::string> dsstring;
	VectorDeserializer<InkListDefinitionEntry> vdsentry;

	name = dsstring(bytes, index);
	entries = vdsentry(bytes, index);

	return this;
}

void InkObjectList::execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	InkList new_list_var;
	//std::vector<InkListDefinitionEntry> list_entries;
	//list_entries.reserve(entries.size());
	for (const InkListDefinitionEntry& entry : entries) {
		//list_entries.emplace_back(entry.label, entry.value);
		new_list_var.add_item(InkListItem(entry.label, entry.value, story_state.variable_info.current_list_definition_uuid));
	}

	//story_state.variable_info.add_list_definition(list_entries);
	story_state.variable_info.variables[name] = new_list_var;
}
