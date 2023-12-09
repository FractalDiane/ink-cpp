#include "objects/ink_object_globalvariable.h"

#include "shunting-yard.h"

void InkObjectGlobalVariable::execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	story_state.variables[name] = cparse::calculator::calculate(value.c_str(), story_state.variables);
}
