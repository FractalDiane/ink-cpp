#include "objects/ink_object_interpolation.h"

#include "shunting-yard.h"

void InkObjectInterpolation::execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	cparse::packToken result = cparse::calculator::calculate(what_to_interpolate.c_str(), story_state.variables);
	eval_result.result += result.str();
}
