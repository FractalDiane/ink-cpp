#include "objects/ink_object_linebreak.h"

void InkObjectLineBreak::execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	eval_result.reached_newline = story_state.current_knot().reached_newline = eval_result.has_any_contents(true);
}
