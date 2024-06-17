#pragma once

#include "objects/ink_object.h"

#include <string>

class InkObjectInterpolation : public InkObject {
private:
	struct ExpressionParserV2::ShuntedExpression what_to_interpolate;

public:
	InkObjectInterpolation(const struct ExpressionParserV2::ShuntedExpression& interpolation) : what_to_interpolate{interpolation} {}
 
	virtual ~InkObjectInterpolation() override;

	virtual ObjectId get_id() const override { return ObjectId::Interpolation; }

	virtual void execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) override;

	virtual ByteVec to_bytes() const override;
	virtual InkObject* populate_from_bytes(const ByteVec& bytes, std::size_t& index) override;

	virtual bool stop_before_this(const InkStoryState& story_state) const override { return what_to_interpolate.stack_empty() || what_to_interpolate.preparation_stack.back().function_eval_index == SIZE_MAX; }
};
