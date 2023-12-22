#include "objects/ink_object_conditional.h"

#include "ink_utils.h"

#include "shunting-yard.h"

InkObjectConditional::~InkObjectConditional() {
	for (InkObject* object : branch_if) {
		delete object;
	}

	for (InkObject* object : branch_else) {
		delete object;
	}
}

void InkObjectConditional::execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	cparse::TokenMap vars = story_state.story_tracking.add_visit_count_variables(story_state.variables, story_state.current_knot().knot, story_state.current_stitch);
	cparse::packToken result = cparse::calculator::calculate(deinkify_expression(condition).c_str(), vars);
	const std::vector<InkObject*>& result_array = result.asBool() ? branch_if : branch_else;
	for (InkObject* object : result_array) {
		object->execute(story_state, eval_result);
	}
}
