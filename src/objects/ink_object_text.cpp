#include "objects/ink_object_text.h"

std::vector<std::uint8_t> InkObjectText::to_bytes() const {
	Serializer<std::string> s;
	return s(text_contents);
}

InkObject* InkObjectText::populate_from_bytes(const std::vector<std::uint8_t>& bytes, std::size_t& index) {
	Serializer<std::string> ds;
	text_contents = ds(bytes, index);
	return this;
}

void InkObjectText::append_text(const std::string& text) {
	text_contents += text;
}
