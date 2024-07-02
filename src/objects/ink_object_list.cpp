#include "objects/ink_object_list.h"

#include "ink_utils.h"
#include "types/ink_list.h"

ByteVec InkObjectList::to_bytes() const {
	Serializer<std::string> sstring;
	Serializer<Uuid> sorigin;
	VectorSerializer<InkListDefinition::Entry> vsentry;
	
	ByteVec result = sstring(name);
	ByteVec result2 = sorigin(list_uuid);
	ByteVec result3 = vsentry(entries);

	result.insert(result.end(), result2.begin(), result2.end());
	result.insert(result.end(), result3.begin(), result3.end());

	return result;
}

InkObject* InkObjectList::populate_from_bytes(const ByteVec& bytes, std::size_t& index) {
	Deserializer<std::string> dsstring;
	Deserializer<Uuid> dsorigin;
	VectorDeserializer<InkListDefinition::Entry> vdsentry;

	name = dsstring(bytes, index);
	list_uuid = dsorigin(bytes, index);
	entries = vdsentry(bytes, index);

	return this;
}

void InkObjectList::execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	InkList new_list_var{story_state.variable_info.defined_lists};
	new_list_var.add_origin(list_uuid);
	for (const InkListDefinition::Entry& entry : entries) {
		if (entry.is_included_by_default) {
			new_list_var.add_item(entry.label);
		}
	}
	
	story_state.variable_info.variables[name] = new_list_var;
}
