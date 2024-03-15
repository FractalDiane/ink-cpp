#include "objects/ink_object.h"

ByteVec Serializer<InkObject*>::operator()(const InkObject* value) {
	return value->get_serialized_bytes();
}

/*InkObject* Deserializer<InkObject*>::operator()(const ByteVec& bytes, std::size_t& index) {
	return nullptr;
}*/

std::string InkObject::to_string() const {
	return "NO TO_STRING";
}

std::vector<std::uint8_t> InkObject::to_bytes() const {
	return {};
}

InkObject* InkObject::populate_from_bytes(const std::vector<std::uint8_t>& bytes, std::size_t& index) {
	return this;
}

ByteVec InkObject::get_serialized_bytes() const {
	ByteVec result = {static_cast<std::uint8_t>(get_id())};
	ByteVec serialization = to_bytes();
	result.insert(result.end(), serialization.begin(), serialization.end());
	return result;
}
