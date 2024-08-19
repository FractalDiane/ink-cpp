#pragma once

#include "objects/ink_object.h"
#include "ink_utils.h"

#include <string>

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
	virtual std::string to_string() const override { return text_contents; }
	virtual bool has_any_contents(bool strip) const override;
	virtual bool contributes_content_to_knot() const override { return true; }

	virtual void execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) override;

	virtual bool stop_before_this(const InkStoryState& story_state) const override { return !strip_string_edges(text_contents, true, true, true).empty(); }

	void append_text(const std::string& text);
	const std::string& get_text_contents() const { return text_contents; }
	void set_text_contents(std::string&& new_contents) { text_contents = new_contents; }
};
