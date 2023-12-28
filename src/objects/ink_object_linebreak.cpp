#include "objects/ink_object_linebreak.h"

void InkObjectLineBreak::execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	InkObject* next_object = story_state.get_current_object(1);
	ObjectId next_object_type = next_object ? next_object->get_id() : static_cast<ObjectId>(-1);
	eval_result.should_continue = eval_result.result.empty() || story_state.in_glue || story_state.just_diverted_to_non_knot || next_object_type == ObjectId::Glue;
	story_state.in_glue = false;
	story_state.choice_mix_position = InkStoryState::ChoiceMixPosition::Before;

	if (!eval_result.should_continue) {
		if (next_object_type == ObjectId::Divert) {
			next_object->execute(story_state, eval_result); // HACK: is there a better way to do this?
			story_state.check_for_glue_divert = true;
		} else if (next_object_type == ObjectId::Choice) { // HACK: is there a better way to do this?
			eval_result.should_continue = !next_object->will_choice_take_fallback(story_state);
		}
	}
}
