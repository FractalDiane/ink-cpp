#pragma once

#include "objects/ink_object.h"

#include <string>

class InkObjectInterpolation : public InkObject {
private:
	std::vector<struct ExpressionParser::Token*> what_to_interpolate;

public:
	InkObjectInterpolation(const std::vector<struct ExpressionParser::Token*>& interpolation) : what_to_interpolate{interpolation} {}
 
	virtual ~InkObjectInterpolation() override;

	virtual ObjectId get_id() const override { return ObjectId::Interpolation; }

	virtual void execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) override;

	virtual bool stop_before_this() const override { return true; }
};
