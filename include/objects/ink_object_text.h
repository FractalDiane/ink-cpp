#pragma once

#include "objects/ink_object.h"

#include <string>
#include <format>

class InkObjectText : public InkObject {
private:
	std::string text_contents;

public:
	InkObjectText() : text_contents{std::string()} {}
	InkObjectText(const std::string& text) : text_contents{text} {}
	InkObjectText(std::string&& text) : text_contents{text} {}

	virtual std::vector<std::uint8_t> to_bytes() const override;
	virtual InkObject* populate_from_bytes(const std::vector<std::uint8_t>& bytes, std::size_t& index) override;
	virtual ObjectId get_id() const override { return ObjectId::Text; }
	virtual std::string to_string() const override { return std::format("Text ({})", text_contents); }
	virtual bool has_any_contents() const override { return !text_contents.empty(); }

	virtual void execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) override;

	void append_text(const std::string& text);
	const std::string& get_text_contents() const { return text_contents; }
};
