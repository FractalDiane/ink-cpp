#include "objects/ink_object_linebreak.h"

void InkObjectLineBreak::execute(InkStoryState& story, InkStoryEvalResult& eval_result) {
	eval_result.should_continue = false;
}
