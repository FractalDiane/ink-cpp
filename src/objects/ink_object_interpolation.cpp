#include "objects/ink_object_interpolation.h"

#include "ink_utils.h"

#include "shunting-yard.h"

void InkObjectInterpolation::execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	cparse::packToken result = cparse::calculator::calculate(deinkify_expression(what_to_interpolate).c_str(), story_state.get_variables_with_locals());
	eval_result.result += result.str();
}
