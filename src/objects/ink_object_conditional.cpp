#include "objects/ink_object_conditional.h"

#include "shunting-yard.h"

void InkObjectConditional::execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	cparse::packToken result = cparse::calculator::calculate(condition.c_str(), story_state.variables);
	const std::vector<InkObject*>& result_array = result.asBool() ? branch_if : branch_else;
	for (InkObject* object : result_array) {
		object->execute(story_state, eval_result);
	}
}
