#pragma once

#include "objects/ink_object.h"

#include <string>

class InkObjectInterpolation : public InkObject {
private:
	std::string what_to_interpolate;

public:
	InkObjectInterpolation(const std::string& interpolation) : what_to_interpolate{interpolation} {}
 
	virtual ObjectId get_id() const override { return ObjectId::Interpolation; }

	virtual void execute(InkStoryData* const story_data, InkStoryState& story_state, InkStoryEvalResult& eval_result) override;
};
