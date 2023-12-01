#include "objects/ink_object_linebreak.h"

void InkObjectLineBreak::execute(InkStoryState& story, InkStoryEvalResult& eval_result) {
	InkObject* next_object = story.get_current_object(1);
	ObjectId next_object_type = next_object ? next_object->get_id() : static_cast<ObjectId>(-1);
	eval_result.should_continue = story.in_glue || next_object_type == ObjectId::Glue || next_object_type == ObjectId::Choice;
}
