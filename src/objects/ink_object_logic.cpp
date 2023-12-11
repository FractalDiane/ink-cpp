#include "objects/ink_object_logic.h"

#include "runtime/ink_story.h"
#include "ink_utils.h"

#include "shunting-yard.h"

void InkObjectLogic::execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	cparse::calculator::calculate(deinkify_expression(contents).c_str(), story_state.variables);
}
