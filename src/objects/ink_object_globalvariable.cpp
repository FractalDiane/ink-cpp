#include "objects/ink_object_globalvariable.h"


void InkObjectGlobalVariable::execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	/*exprtk::symbol_table<double> symbol_table;
	exprtk::expression<double> expression;
	exprtk::parser<double> parser;

	for (const auto& entry : story_state.knot_visit_counts) {
		symbol_table.add_constant(entry.first, static_cast<double>(entry.second));
	}

	expression.register_symbol_table(symbol_table);
	parser.compile(value, expression);

	std::vector<std::pair<std::string, double>> list;
	symbol_table.get_variable_list(list);*/
}
