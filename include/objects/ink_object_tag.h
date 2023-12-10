#pragma once

#include "objects/ink_object.h"

#include <string>
#include <format>

class InkObjectTag : public InkObject {
private:
	std::string tag;

public:
	InkObjectTag(const std::string& tag_text) : tag{tag_text} {}

	virtual ObjectId get_id() const override { return ObjectId::Tag; }
	virtual std::vector<std::uint8_t> to_bytes() const override;
	virtual InkObject* populate_from_bytes(const std::vector<std::uint8_t>& bytes, std::size_t& index) override;
	virtual bool has_any_contents(bool strip) const override { return !tag.empty(); }

	virtual void execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) override;
	
	virtual std::string to_string() const override { return std::format("Tag ({})", tag); }
};
