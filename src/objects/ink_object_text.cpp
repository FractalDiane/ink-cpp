#include "objects/ink_object_text.h"

std::vector<std::uint8_t> InkObjectText::to_bytes() const {
	Serializer<std::string> s;
	return s(text_contents);
}

void InkObjectText::append_text(const std::string& text) {
	text_contents += text;
}
