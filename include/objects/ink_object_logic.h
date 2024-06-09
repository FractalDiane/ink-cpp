#pragma once

#include "objects/ink_object.h"

#include <string>

class InkObjectLogic : public InkObject {
private:
	struct ExpressionParser::ShuntedExpression contents_shunted_tokens;

public:
	InkObjectLogic(const struct ExpressionParser::ShuntedExpression& contents_shunted_tokens) : contents_shunted_tokens{contents_shunted_tokens} {}

	virtual ~InkObjectLogic() override;

	virtual ObjectId get_id() const override { return ObjectId::Logic; }
	//virtual std::string to_string() const override { return contents; }

	virtual ByteVec to_bytes() const override;
	virtual InkObject* populate_from_bytes(const ByteVec& bytes, std::size_t& index) override;

	virtual void execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) override;
};
