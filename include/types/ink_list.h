#pragma once

#include "uuid.h"
#include "serialization.h"

#include <string>
#include <vector>
#include <initializer_list>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <optional>

class InkListDefinition {
public:
	struct Entry {
		std::string label;
		std::int64_t value = 0;
		bool is_included_by_default = false;
	};

private:
	std::string name;
	std::unordered_map<std::string, std::int64_t> list_entries;
	Uuid uuid;

	friend class InkList;

public:
	InkListDefinition(const std::string& name, std::initializer_list<std::string> entries, Uuid uuid) : name(name), uuid(uuid) {
		std::int64_t index = 1;
		for (const std::string& entry : entries) {
			list_entries.emplace(entry, index++);
		}
	}

	InkListDefinition(const std::string& name, const std::vector<Entry>& entries, Uuid uuid) : name(name), uuid(uuid) {
		for (const Entry& entry : entries) {
			list_entries.emplace(entry.label, entry.value);
		}
	}

	std::optional<std::int64_t> get_entry_value(const std::string& entry) const;
	std::optional<std::string> get_label_from_value(std::int64_t value) const;
	InkList get_sublist_from_value(std::int64_t value, const struct InkListDefinitionMap* definition_map) const;

	inline Uuid get_uuid() const { return uuid; }
	inline const std::string& get_name() const { return name; }
};

struct InkListDefinitionMap {
	std::unordered_map<Uuid, InkListDefinition> defined_lists;
	UuidValue current_list_definition_uuid = 0;

	Uuid add_list_definition(const std::string& name, const std::vector<InkListDefinition::Entry>& values);
	std::optional<Uuid> get_list_entry_origin(const std::string& entry) const;
	bool contains_list_name(const std::string& name) const;
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

	bool operator<(const InkListItem& other) const {
		if (value != other.value) {
			return value < other.value;
		} else if (origin_list_uuid != other.origin_list_uuid) {
			return origin_list_uuid.get() < other.origin_list_uuid.get();
		} else {
			return label < other.label;
		}
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
	std::set<InkListItem> current_values;
	std::unordered_set<Uuid> all_origins;
	const InkListDefinitionMap* owning_definition_map;

public:
	InkList() : current_values{}, owning_definition_map(nullptr) {}

	InkList(const InkStory& story);
	InkList(const InkListDefinitionMap& definition_map);
	InkList(const InkListDefinitionMap* definition_map);

	InkList(std::initializer_list<std::string> current_values, const InkStory& story);
	InkList(std::initializer_list<std::string> current_values, const InkListDefinitionMap& definition_map);
	InkList(std::initializer_list<std::string> current_values, const InkListDefinitionMap* definition_map);

	InkList(std::initializer_list<InkListItem> current_values, const InkStory& story);
	InkList(std::initializer_list<InkListItem> current_values, const InkListDefinitionMap& definition_map);
	InkList(std::initializer_list<InkListItem> current_values, const InkListDefinitionMap* definition_map);
	
	InkList(const InkList& from);
	InkList& operator=(const InkList& from);

	void add_item(const std::string& item_name);
	void add_item(const InkListItem& item);
	void remove_item(const std::string& item_name);
	void remove_item(const InkListItem& item);

	void add_origin(Uuid origin) { all_origins.insert(origin); }

	InkList union_with(const InkList& other) const;
	InkList intersect_with(const InkList& other) const;
	InkList without(const InkList& other) const;
	bool contains(const InkList& other) const;
	InkList inverted() const;

	std::int64_t value() const;

	std::size_t count() const { return current_values.size(); }
	std::size_t size() const { return current_values.size(); }
	bool empty() const { return current_values.empty(); }
	InkListItem single_item() const;
	InkListItem min_item() const;
	InkListItem max_item() const;
	InkList min() const;
	InkList max() const;
	InkList at(std::size_t index) const;
	InkList all_possible_items() const;
	InkList range(std::int64_t from, std::int64_t to) const;
	InkList range(const InkList& from, const InkList& to) const;

	auto begin() { return current_values.begin(); }
	auto end() { return current_values.end(); }
	auto cbegin() const { return current_values.cbegin(); }
	auto cend() const { return current_values.cend(); }

	bool operator==(const InkList& other) const { return current_values == other.current_values; }
	bool operator!=(const InkList& other) const { return current_values != other.current_values; }
	bool operator<(const InkList& other) const { return max_item().value < other.min_item().value; }
	bool operator>(const InkList& other) const { return min_item().value > other.max_item().value; }
	bool operator<=(const InkList& other) const { return min_item().value <= other.min_item().value && max_item().value <= other.max_item().value; }
	bool operator>=(const InkList& other) const { return min_item().value >= other.min_item().value && max_item().value >= other.max_item().value; }

	InkList operator+(const InkList& other) const { return union_with(other); }
	InkList operator-(const InkList& other) const { return without(other); }
	InkList operator^(const InkList& other) const { return intersect_with(other); }

	InkList operator+(std::int64_t amount) const;
	InkList operator-(std::int64_t amount) const;

	// HACK: MAKE THESE LESS BAD
	InkList& operator+=(const InkList& other) { *this = *this + other; return *this; }
	InkList& operator-=(const InkList& other) { *this = *this - other; return *this; }
	InkList& operator^=(const InkList& other) { *this = *this ^ other; return *this; }
	InkList& operator+=(std::int64_t amount) { *this = *this + amount; return *this; }
	InkList& operator-=(std::int64_t amount) { *this = *this - amount; return *this; }

	InkList& operator++(int) { *this += 1; return *this; }
	InkList& operator--(int) { *this -= 1; return *this; }

	ByteVec to_bytes() const;
	static InkList from_bytes(const ByteVec& bytes, std::size_t& index);
};

template <>
struct Serializer<InkListItem> {
	ByteVec operator()(const InkListItem& item);
};

template <>
struct Deserializer<InkListItem> {
	InkListItem operator()(const ByteVec& bytes, std::size_t& index);
};

template <>
struct Serializer<InkList> {
	ByteVec operator()(const InkList& list);
};

template <>
struct Deserializer<InkList> {
	InkList operator()(const ByteVec& bytes, std::size_t& index);
};

template <>
struct Serializer<InkListDefinition::Entry> {
	ByteVec operator()(const InkListDefinition::Entry& entry);
};

template <>
struct Deserializer<InkListDefinition::Entry> {
	InkListDefinition::Entry operator()(const ByteVec& bytes, std::size_t& index);
};
