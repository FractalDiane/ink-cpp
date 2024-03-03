#pragma once

#include "objects/ink_object.h"

#include <string>

class InkObjectLogic : public InkObject {
private:
	std::string contents;
	std::vector<struct ExpressionParser::Token*> contents_shunted_tokens;

public:
	InkObjectLogic(const std::string& contents, const std::vector<struct ExpressionParser::Token*>& contents_shunted_tokens) : contents{contents}, contents_shunted_tokens{contents_shunted_tokens} {}

	virtual ~InkObjectLogic() override;

	virtual ObjectId get_id() const override { return ObjectId::Logic; }
	virtual std::string to_string() const override { return contents; }

	virtual void execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) override;
};
