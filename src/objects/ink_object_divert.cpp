#include "objects/ink_object_divert.h"

std::vector<std::uint8_t> InkObjectDivert::to_bytes() const {
	Serializer<std::string> s;
	return s(target_knot);
}

InkObject* InkObjectDivert::populate_from_bytes(const std::vector<std::uint8_t>& bytes, std::size_t& index) {
	Serializer<std::string> ds;
	target_knot = ds(bytes, index);
	return this;
}
