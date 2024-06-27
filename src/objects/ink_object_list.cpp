#include "objects/ink_object_list.h"

#include "ink_utils.h"
#include "types/ink_list.h"

ByteVec InkObjectList::to_bytes() const {
	Serializer<std::string> sstring;
	VectorSerializer<InkListDefinition::Entry> vsentry;
	
	ByteVec result = sstring(name);
	ByteVec result2 = vsentry(entries);

	result.insert(result.end(), result2.begin(), result2.end());

	return result;
}

InkObject* InkObjectList::populate_from_bytes(const ByteVec& bytes, std::size_t& index) {
	Deserializer<std::string> dsstring;
	VectorDeserializer<InkListDefinition::Entry> vdsentry;

	name = dsstring(bytes, index);
	entries = vdsentry(bytes, index);

	return this;
}

void InkObjectList::execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	InkList new_list_var{story_state.variable_info.defined_lists};
	story_state.variable_info.variables[name] = new_list_var;
}
