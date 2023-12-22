#include "objects/ink_object_globalvariable.h"

#include "ink_utils.h"

#include "shunting-yard.h"

void InkObjectGlobalVariable::execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	story_state.variables[name] = cparse::calculator::calculate(deinkify_expression(value).c_str(), story_state.get_variables_with_locals());
}
