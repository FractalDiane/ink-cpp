#include "types/ink_list.h"

#include "runtime/ink_story.h"

std::optional<std::int64_t> InkListDefinition::get_entry_value(const std::string& entry) const {
	if (auto found_entry = list_entries.find(entry); found_entry != list_entries.end()) {
		return found_entry->second;
	}

	return std::nullopt;
}

std::optional<std::string> InkListDefinition::get_label_from_value(std::int64_t value) const {
	for (const auto& entry : list_entries) {
		if (entry.second == value) {
			return entry.first;
		}
	}

	return std::nullopt;
}

void InkListDefinitionMap::add_list_definition(const std::vector<InkListDefinition::Entry>& values) {
	Uuid new_uuid = current_list_definition_uuid++;
	defined_lists.emplace(new_uuid, InkListDefinition(values, new_uuid));
}

std::optional<Uuid> InkListDefinitionMap::get_list_entry_origin(const std::string& entry) const {
	for (const auto& list : defined_lists) {
		if (list.second.get_entry_value(entry).has_value()) {
			return list.first;
		}
	}

	return std::nullopt;
}

InkList::InkList(const InkStory& story) : current_values{}, owning_definition_map(&story.get_story_state().variable_info.defined_lists) {}
InkList::InkList(const InkListDefinitionMap& definition_map) : current_values{}, owning_definition_map(&definition_map) {}
InkList::InkList(const InkListDefinitionMap* definition_map) : current_values{}, owning_definition_map(definition_map) {}

InkList::InkList(std::initializer_list<std::string> current_values, const InkStory& story) : current_values{}, owning_definition_map(&story.get_story_state().variable_info.defined_lists) {
	for (const std::string& string : current_values) {
		add_item(string);
	}
}

InkList::InkList(std::initializer_list<std::string> current_values, const InkListDefinitionMap& definition_map) : current_values{}, owning_definition_map(&definition_map) {
	for (const std::string& string : current_values) {
		add_item(string);
	}
}

InkList::InkList(std::initializer_list<std::string> current_values, const InkListDefinitionMap* definition_map) : current_values{}, owning_definition_map(definition_map) {
	for (const std::string& string : current_values) {
		add_item(string);
	}
}

InkList::InkList(std::initializer_list<InkListItem> current_values, const InkStory& story) : current_values(current_values), owning_definition_map(&story.get_story_state().variable_info.defined_lists) {}
InkList::InkList(std::initializer_list<InkListItem> current_values, const InkListDefinitionMap& definition_map) : current_values(current_values), owning_definition_map(&definition_map) {}
InkList::InkList(std::initializer_list<InkListItem> current_values, const InkListDefinitionMap* definition_map) : current_values(current_values), owning_definition_map(definition_map) {}

InkList::InkList(const InkList& from) : current_values(from.current_values), owning_definition_map(from.owning_definition_map) {}

InkList& InkList::operator=(const InkList& from) {
	if (this != &from) {
		current_values = from.current_values;
		owning_definition_map = from.owning_definition_map;
	}

	return *this;
}

void InkList::add_item(const std::string& item_name) {
	for (const auto& list : owning_definition_map->defined_lists) {
		if (std::optional<std::int64_t> entry_value = list.second.get_entry_value(item_name); entry_value.has_value()) {
			current_values.insert(InkListItem(item_name, *entry_value, list.second.get_uuid()));
			all_origins.insert(list.second.get_uuid());
			return;
		}
	}
}

void InkList::add_item(const InkListItem& item) {
	current_values.insert(item);
	all_origins.insert(item.origin_list_uuid);
}

void InkList::remove_item(const std::string& item_name) {
	for (const auto& list : owning_definition_map->defined_lists) {
		if (std::optional<std::int64_t> entry_value = list.second.get_entry_value(item_name); entry_value.has_value()) {
			current_values.erase(InkListItem(item_name, *entry_value, list.second.get_uuid()));
			return;
		}
	}
}

void InkList::remove_item(const InkListItem& item) {
	current_values.erase(item);
}

InkList InkList::union_with(const InkList& other) const {
	InkList result = *this;
	for (const InkListItem& item : other.current_values) {
		result.add_item(item);
	}

	return result;
}

InkList InkList::intersect_with(const InkList& other) const {
	InkList result{owning_definition_map};
	for (const InkListItem& item : other.current_values) {
		if (current_values.contains(item)) {
			result.add_item(item);
		}
	}

	return result;
}

InkList InkList::without(const InkList& other) const {
	InkList result = *this;
	for (const InkListItem& item : other.current_values) {
		result.remove_item(item);
	}

	return result;
}

bool InkList::contains(const InkList& other) const {
	for (const InkListItem& item : other.current_values) {
		if (!current_values.contains(item)) {
			return false;
		}
	}

	return true;
}

InkList InkList::inverse() const {
	InkList result{owning_definition_map};
	for (Uuid origin : all_origins) {
		const InkListDefinition& this_definition = owning_definition_map->defined_lists.at(origin);
		for (const auto& entry : this_definition.list_entries) {
			if (!current_values.contains(InkListItem(entry.first, entry.second, this_definition.get_uuid()))) {
				result.add_item(InkListItem(entry.first, entry.second, origin));
			}
		}
	}

	return result;
}

InkListItem InkList::min_item() const {
	if (current_values.empty()) {
		return InkListItem();
	} else if (current_values.size() == 1) {
		return *current_values.begin();
	}

	std::int64_t min_value = INT64_MAX;
	const InkListItem* min_item = nullptr;
	for (const InkListItem& item : current_values) {
		if (item.value < min_value) {
			min_value = item.value;
			min_item = &item;
		}
	}

	return *min_item;
}

InkListItem InkList::max_item() const {
	if (current_values.empty()) {
		return InkListItem();
	} else if (current_values.size() == 1) {
		return *current_values.begin();
	}

	std::int64_t max_value = INT64_MIN;
	const InkListItem* max_item = nullptr;
	for (const InkListItem& item : current_values) {
		if (item.value > max_value) {
			max_value = item.value;
			max_item = &item;
		}
	}

	return *max_item;
}

InkList InkList::all_possible_items() const {
	InkList result{owning_definition_map};
	for (Uuid origin : all_origins) {
		const InkListDefinition& this_definition = owning_definition_map->defined_lists.at(origin);
		for (const auto& entry : this_definition.list_entries) {
			result.add_item(InkListItem(entry.first, entry.second, origin));
		}
	}

	return result;
}

InkList InkList::operator+(std::int64_t amount) const {
	InkList result{owning_definition_map};
	for (const InkListItem& item : current_values) {
		if (std::optional<std::string> incremented_label = owning_definition_map->defined_lists.at(item.origin_list_uuid).get_label_from_value(item.value + amount); incremented_label.has_value()) {
			result.add_item(*incremented_label);
		}
	}

	return result;
}

InkList InkList::operator-(std::int64_t amount) const {
	InkList result{owning_definition_map};
	for (const InkListItem& item : current_values) {
		if (std::optional<std::string> incremented_label = owning_definition_map->defined_lists.at(item.origin_list_uuid).get_label_from_value(item.value - amount); incremented_label.has_value()) {
			result.add_item(*incremented_label);
		}
	}

	return result;
}

ByteVec InkList::to_bytes() const {
	std::vector<InkListItem> values;
	values.reserve(current_values.size());
	for (const InkListItem& value : current_values) {
		values.push_back(value);
	}

	/*std::vector<Uuid> origins;
	origins.reserve(all_origins.size());
	for (Uuid origin : all_origins) {
		origins.push_back(origin);
	}*/

	VectorSerializer<InkListItem> sitems;
	//VectorSerializer<Uuid> sorigins;

	ByteVec result = sitems(values);
	//ByteVec result2 = sorigins(origins);
	//result.insert(result.end(), result2.begin(), result2.end());
	return result;
}

InkList InkList::from_bytes(const ByteVec& bytes, std::size_t& index) {
	VectorDeserializer<InkListItem> dsitems;
	//VectorDeserializer<Uuid> dsorigins;

	std::vector<InkListItem> items = dsitems(bytes, index);
	//std::vector<Uuid> origins = dsorigins(bytes, index);

	InkList result;
	for (const InkListItem& item : items) {
		result.add_item(item);
	}

	return result;
}

ByteVec Serializer<InkListItem>::operator()(const InkListItem& item) {
	Serializer<std::string> sstring;
	Serializer<std::int64_t> svalue;
	Serializer<Uuid> sorigin;

	ByteVec result = sstring(item.label);
	ByteVec result2 = svalue(item.value);
	ByteVec result3 = sorigin(item.origin_list_uuid);

	result.insert(result.end(), result2.begin(), result2.end());
	result.insert(result.end(), result3.begin(), result3.end());
	return result;
}

InkListItem Deserializer<InkListItem>::operator()(const ByteVec& bytes, std::size_t& index) {
	Deserializer<std::string> dsstring;
	Deserializer<std::int64_t> dsvalue;
	Deserializer<Uuid> dsorigin;

	std::string label = dsstring(bytes, index);
	std::int64_t value = dsvalue(bytes, index);
	Uuid origin = dsorigin(bytes, index);

	return InkListItem(label, value, origin);
}

ByteVec Serializer<InkList>::operator()(const InkList& list) {
	return list.to_bytes();
}

InkList Deserializer<InkList>::operator()(const ByteVec& bytes, std::size_t& index) {
	return InkList::from_bytes(bytes, index);
}

ByteVec Serializer<InkListDefinition::Entry>::operator()(const InkListDefinition::Entry& entry) {
	Serializer<std::string> sstring;
	Serializer<std::int64_t> s64;

	ByteVec result = sstring(entry.label);
	ByteVec result2 = s64(entry.value);
	result.insert(result.end(), result2.begin(), result2.end());
	result.push_back(static_cast<std::uint8_t>(entry.is_included_by_default));
	return result;
}

InkListDefinition::Entry Deserializer<InkListDefinition::Entry>::operator()(const ByteVec& bytes, std::size_t& index) {
	Deserializer<std::string> dsstring;
	Deserializer<std::int64_t> ds64;
	Deserializer<std::uint8_t> ds8;

	InkListDefinition::Entry result;
	result.label = dsstring(bytes, index);
	result.value = ds64(bytes, index);
	result.is_included_by_default = static_cast<bool>(ds8(bytes, index));

	return result;
}
