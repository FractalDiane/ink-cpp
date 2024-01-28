#include "objects/ink_object_conditional.h"

#include "ink_utils.h"

#include "shunting-yard.h"

/*InkObjectConditional::InkObjectConditional(const std::vector<std::pair<std::string, std::vector<InkObject*>>>& branches, const std::vector<InkObject*>& objects_else)
: is_switch{false}
{
	for (const auto& entry : branches) {
		Knot this_knot{entry.second};
		this->branches.push_back({entry.first, this_knot});
	}
}

InkObjectConditional::InkObjectConditional(const std::string& switch_expression, const std::vector<std::pair<std::string, std::vector<InkObject*>>>& branches, const std::vector<InkObject*>& objects_else)
: is_switch{true}
{

}*/

InkObjectConditional::~InkObjectConditional() {
	for (auto& entry : branches) {
		for (InkObject* object : entry.second.objects) {
			delete object;
		}
	}

	for (InkObject* object : branch_else.objects) {
		delete object;
	}
}

void InkObjectConditional::execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	cparse::TokenMap vars = story_state.story_tracking.add_visit_count_variables(story_state.variables, story_state.current_knot().knot, story_state.current_stitch);
	
	if (!is_switch) {
		for (auto& entry : branches) {
			cparse::packToken condition_result = cparse::calculator::calculate(deinkify_expression(entry.first).c_str(), vars);
			if (condition_result.asBool()) {
				/*for (InkObject* object : entry.second) {
					object->execute(story_state, eval_result);
				}*/

				story_state.current_knots_stack.push_back({&(entry.second), 0});

				return;
			}
		}
	} else {
		// TODO: this might be redundant and strictly worse performance than the above version
		cparse::packToken result = cparse::calculator::calculate(deinkify_expression(switch_expression).c_str(), vars);
		for (auto& entry : branches) {
			cparse::packToken condition_result = cparse::calculator::calculate(deinkify_expression(entry.first).c_str(), vars);
			if (condition_result == result) {
				/*for (InkObject* object : entry.second) {
					object->execute(story_state, eval_result);
				}*/

				story_state.current_knots_stack.push_back({&(entry.second), 0});

				return;
			}
		}
	}
	
	
	/*for (InkObject* object : branch_else) {
		object->execute(story_state, eval_result);
	}*/

	story_state.current_knots_stack.push_back({&branch_else, 0});
}
