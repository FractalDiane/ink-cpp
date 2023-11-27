#include "objects/ink_object.h"

std::string InkObject::to_string() const {
	return "NO TO_STRING";
}

std::vector<std::uint8_t> InkObject::to_bytes() const {
	return {};
}

InkObject* InkObject::populate_from_bytes(const std::vector<std::uint8_t>& bytes, std::size_t& index) {
	return this;
}

std::vector<std::uint8_t> InkObject::get_serialized_bytes() const {
	std::vector<std::uint8_t> result = {static_cast<std::uint8_t>(get_id())};
	std::vector<std::uint8_t> serialization = to_bytes();
	result.insert(result.end(), serialization.begin(), serialization.end());
	return result;
}
