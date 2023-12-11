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
	cparse::packToken result = cparse::calculator::calculate(deinkify_expression(condition).c_str(), story_state.variables);
	const std::vector<InkObject*>& result_array = result.asBool() ? branch_if : branch_else;
	for (InkObject* object : result_array) {
		object->execute(story_state, eval_result);
	}
}
