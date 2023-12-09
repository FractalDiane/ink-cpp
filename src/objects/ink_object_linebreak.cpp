#include "objects/ink_object_linebreak.h"

void InkObjectLineBreak::execute(InkStoryState& story, InkStoryEvalResult& eval_result) {
	InkObject* next_object = story.get_current_object(1);
	ObjectId next_object_type = next_object ? next_object->get_id() : static_cast<ObjectId>(-1);
	eval_result.should_continue = eval_result.result.empty() || story.in_glue || next_object_type == ObjectId::Glue;
	story.in_glue = false;

	if (!eval_result.should_continue) {
		if (next_object_type == ObjectId::Divert) {
			next_object->execute(story, eval_result); // HACK: is there a better way to do this?
			story.check_for_glue_divert = true;
		} else if (next_object_type == ObjectId::Choice) { // HACK: is there a better way to do this?
			eval_result.should_continue = !next_object->will_choice_take_fallback(story);
		}
	}
}
