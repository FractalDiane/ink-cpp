#include "ink_cparse_ext.h"

#include <cmath>

cparse::packToken op_substr(const cparse::packToken& left, const cparse::packToken& right, cparse::evaluationData* data) {
	return left.asString().contains(right.asString());
}

/*cparse::packToken op_increment_prefix(const cparse::packToken& left, const cparse::packToken& right, cparse::evaluationData* data) {
	return right.asDouble() + 1;
}

cparse::packToken op_increment_postfix(const cparse::packToken& left, const cparse::packToken& right, cparse::evaluationData* data) {
	return left.asDouble() + 1;
}

cparse::packToken op_decrement_prefix(const cparse::packToken& left, const cparse::packToken& right, cparse::evaluationData* data) {
	return right.asDouble() - 1;
}

cparse::packToken op_decrement_postfix(const cparse::packToken& left, const cparse::packToken& right, cparse::evaluationData* data) {
	return left.asDouble() - 1;
}*/

cparse::packToken op_mod_int_kw(const cparse::packToken& left, const cparse::packToken& right, cparse::evaluationData* data) {
	return left.asInt() % right.asInt();
}

cparse::packToken op_mod_float_kw(const cparse::packToken& left, const cparse::packToken& right, cparse::evaluationData* data) {
	return std::fmod(left.asDouble(), right.asDouble());
}

void op_mod_parser(const char* expr, const char** rest, cparse::rpnBuilder* data) {
	data->handle_op("mod");
}

InkCparseStartup::InkCparseStartup() {
	cparse::OppMap_t& op_precedence = cparse::calculator::Default().opPrecedence;
	op_precedence.add("?", 2);
	op_precedence.addUnary("++", 2);
	op_precedence.addUnary("--", 2);
	op_precedence.addRightUnary("++", 2);
	op_precedence.addRightUnary("--", 2);
	op_precedence.add("mod", 3);

	cparse::opMap_t& op_map = cparse::calculator::Default().opMap;
	op_map.add({cparse::STR, "?", cparse::STR}, &op_substr);
	op_map.add({cparse::INT, "mod", cparse::INT}, &op_mod_int_kw);
	op_map.add({cparse::REAL, "mod", cparse::REAL}, &op_mod_float_kw);
	/*op_map.add({cparse::UNARY, "++", cparse::NUM}, &op_increment_prefix);
	op_map.add({cparse::NUM, "++", cparse::UNARY}, &op_increment_postfix);
	op_map.add({cparse::UNARY, "--", cparse::NUM}, &op_decrement_prefix);
	op_map.add({cparse::NUM, "--", cparse::UNARY}, &op_decrement_postfix);*/
}

InkCparseStartupParser::InkCparseStartupParser() {
	cparse::parserMap_t& parser = cparse::calculator::Default().parserMap;
	parser.add("mod", &op_mod_parser);
}
