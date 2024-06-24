#pragma once

#include "uuid.h"

#include <string>
#include <vector>
#include <initializer_list>
#include <unordered_set>
#include <unordered_map>
#include <optional>

struct InkListDefinitionEntry {
	std::string label;
	std::int64_t value;
};

class InkListDefinition {
private:
	std::unordered_map<std::string, std::int64_t> list_entries;
	Uuid uuid;

	friend class InkList;

public:
	InkListDefinition(std::initializer_list<std::string> entries, Uuid uuid) : uuid(uuid) {
		std::int64_t index = 1;
		for (const std::string& entry : entries) {
			list_entries.emplace(entry, index++);
		}
	}

	InkListDefinition(const std::vector<InkListDefinitionEntry>& entries, Uuid uuid) : uuid(uuid) {
		for (const InkListDefinitionEntry& entry : entries) {
			list_entries.emplace(entry.label, entry.value);
		}
	}

	std::optional<std::int64_t> get_entry_value(const std::string& entry) const;

	inline Uuid get_uuid() const { return uuid; }
};

struct InkListItem {
	std::string label;
	std::int64_t value;
	Uuid origin_list_uuid;

	friend class InkList;

	InkListItem() : label{}, value(0), origin_list_uuid(0) {}
	InkListItem(const std::string& label, std::int64_t value, Uuid origin_list_uuid) : label(label), value(value), origin_list_uuid(origin_list_uuid) {}

	bool operator==(const InkListItem& other) const {
		return origin_list_uuid == other.origin_list_uuid && label == other.label;
	}

	bool operator!=(const InkListItem& other) const {
		return origin_list_uuid != other.origin_list_uuid || label != other.label;
	}
};

template <>
struct std::hash<InkListItem> {
	std::size_t operator()(const InkListItem& list_item) const noexcept {
		std::size_t h1 = std::hash<std::string>()(list_item.label);
		std::size_t h2 = std::hash<std::int64_t>()(list_item.value);
		return h1 ^ h2;
	}
};

class InkStory;
struct InkStoryState;

class InkList {
	std::unordered_set<InkListItem> current_values;
	std::unordered_set<Uuid> all_origins;
	const InkStoryState* owning_story_state;

public:
	InkList() : current_values{}, owning_story_state(nullptr) {}

	InkList(const InkStory& story);
	InkList(const InkStoryState& story_state);
	InkList(const InkStoryState* story_state);

	InkList(std::initializer_list<std::string> current_values, const InkStory& story);
	InkList(std::initializer_list<std::string> current_values, const InkStoryState& story_state);
	InkList(std::initializer_list<std::string> current_values, const InkStoryState* story_state);

	InkList(std::initializer_list<InkListItem> current_values, const InkStory& story);
	InkList(std::initializer_list<InkListItem> current_values, const InkStoryState& story_state);
	InkList(std::initializer_list<InkListItem> current_values, const InkStoryState* story_state);
	
	InkList(const InkList& from);
	InkList& operator=(const InkList& from);

	void add_item(const std::string& item_name);
	void add_item(const InkListItem& item);
	void remove_item(const std::string& item_name);
	void remove_item(const InkListItem& item);

	InkList union_with(const InkList& other) const;
	InkList intersect_with(const InkList& other) const;
	InkList without(const InkList& other) const;
	bool contains(const InkList& other) const;
	InkList inverse() const;

	InkListItem min_item() const;
	InkListItem max_item() const;
	InkList all_possible_items() const;

	bool operator==(const InkList& other) const { return current_values == other.current_values; }
	bool operator!=(const InkList& other) const { return current_values != other.current_values; }

	InkList operator+(const InkList& other) const { return union_with(other); }
	InkList operator-(const InkList& other) const { return without(other); }
	InkList operator^(const InkList& other) const { return intersect_with(other); }
	//InkList& operator+=(const InkList& other);
	//InkList& operator-=(const InkList& other);
	//InkList& operator^=(const InkList& other);
};
