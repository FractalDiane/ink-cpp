#pragma once

#include "shunting-yard.h"

cparse::packToken op_substr(const cparse::packToken& left, const cparse::packToken& right, cparse::evaluationData* data);
/*cparse::packToken op_increment_prefix(const cparse::packToken& left, const cparse::packToken& right, cparse::evaluationData* data);
cparse::packToken op_increment_postfix(const cparse::packToken& left, const cparse::packToken& right, cparse::evaluationData* data);
cparse::packToken op_decrement_prefix(const cparse::packToken& left, const cparse::packToken& right, cparse::evaluationData* data);
cparse::packToken op_decrement_postfix(const cparse::packToken& left, const cparse::packToken& right, cparse::evaluationData* data);*/
cparse::packToken op_mod_int_kw(const cparse::packToken& left, const cparse::packToken& right, cparse::evaluationData* data);
cparse::packToken op_mod_float_kw(const cparse::packToken& left, const cparse::packToken& right, cparse::evaluationData* data);
void op_mod_parser(const char* expr, const char** rest, cparse::rpnBuilder* data);

struct InkCparseStartup {
	InkCparseStartup();
};

struct InkCparseStartupParser {
	InkCparseStartupParser();
};
