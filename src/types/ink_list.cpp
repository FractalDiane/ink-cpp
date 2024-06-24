#include "types/ink_list.h"

#include "runtime/ink_story.h"

std::optional<std::int64_t> InkListDefinition::get_entry_value(const std::string& entry) const {
	if (auto found_entry = list_entries.find(entry); found_entry != list_entries.end()) {
		return found_entry->second;
	}

	return std::nullopt;
}

InkList::InkList(const InkStory& story) : current_values{}, owning_story_state(&story.get_story_state()) {}
InkList::InkList(const InkStoryState& story_state) : current_values{}, owning_story_state(&story_state) {}
InkList::InkList(const InkStoryState* story_state) : current_values{}, owning_story_state(story_state) {}

InkList::InkList(std::initializer_list<std::string> current_values, const InkStory& story) : current_values{}, owning_story_state(&story.get_story_state()) {
	for (const std::string& string : current_values) {
		add_item(string);
	}
}

InkList::InkList(std::initializer_list<std::string> current_values, const InkStoryState& story_state) : current_values{}, owning_story_state(&story_state) {
	for (const std::string& string : current_values) {
		add_item(string);
	}
}

InkList::InkList(std::initializer_list<std::string> current_values, const InkStoryState* story_state) : current_values{}, owning_story_state(story_state) {
	for (const std::string& string : current_values) {
		add_item(string);
	}
}

InkList::InkList(std::initializer_list<InkListItem> current_values, const InkStory& story) : current_values(current_values), owning_story_state(&story.get_story_state()) {}
InkList::InkList(std::initializer_list<InkListItem> current_values, const InkStoryState& story_state) : current_values(current_values), owning_story_state(&story_state) {}
InkList::InkList(std::initializer_list<InkListItem> current_values, const InkStoryState* story_state) : current_values(current_values), owning_story_state(story_state) {}

InkList::InkList(const InkList& from) : current_values(from.current_values), owning_story_state(from.owning_story_state) {}

InkList& InkList::operator=(const InkList& from) {
	if (this != &from) {
		current_values = from.current_values;
		owning_story_state = from.owning_story_state;
	}

	return *this;
}

void InkList::add_item(const std::string& item_name) {
	for (const auto& list : owning_story_state->variable_info.defined_lists) {
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
	for (const auto& list : owning_story_state->variable_info.defined_lists) {
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
	InkList result{owning_story_state};
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
	InkList result{owning_story_state};
	for (Uuid origin : all_origins) {
		const InkListDefinition& this_definition = owning_story_state->variable_info.defined_lists.at(origin);
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
	InkList result{owning_story_state};
	for (Uuid origin : all_origins) {
		const InkListDefinition& this_definition = owning_story_state->variable_info.defined_lists.at(origin);
		for (const auto& entry : this_definition.list_entries) {
			result.add_item(InkListItem(entry.first, entry.second, origin));
		}
	}

	return result;
}
