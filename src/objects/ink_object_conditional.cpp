#include "objects/ink_object_conditional.h"

#include "ink_utils.h"

#include "shunting-yard.h"

InkObjectConditional::~InkObjectConditional() {
	for (auto& entry : branches) {
		for (InkObject* object : entry.second) {
			delete object;
		}
	}

	for (InkObject* object : branch_else) {
		delete object;
	}
}

void InkObjectConditional::execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	cparse::TokenMap vars = story_state.story_tracking.add_visit_count_variables(story_state.variables, story_state.current_knot().knot, story_state.current_stitch);
	
	if (!is_switch) {
		for (const auto& entry : branches) {
			cparse::packToken condition_result = cparse::calculator::calculate(deinkify_expression(entry.first).c_str(), vars);
			if (condition_result.asBool()) {
				for (InkObject* object : entry.second) {
					object->execute(story_state, eval_result);
				}

				return;
			}
		}
	} else {
		// TODO: this might be redundant and strictly worse performance than the above version
		cparse::packToken result = cparse::calculator::calculate(deinkify_expression(switch_expression).c_str(), vars);
		for (const auto& entry : branches) {
			cparse::packToken condition_result = cparse::calculator::calculate(deinkify_expression(entry.first).c_str(), vars);
			if (condition_result == result) {
				for (InkObject* object : entry.second) {
					object->execute(story_state, eval_result);
				}

				return;
			}
		}
	}
	
	
	for (InkObject* object : branch_else) {
		object->execute(story_state, eval_result);
	}
}
