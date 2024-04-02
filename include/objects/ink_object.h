#pragma once

#include <string>
#include <vector>
#include <utility>

#include "serialization.h"
#include "runtime/ink_story_state.h"
#include "expression_parser/expression_parser.h"

enum class ObjectId {
	Text,
	Choice,
	LineBreak,
	Glue,
	Divert,
	Interpolation,
	Conditional,
	Sequence,
	ChoiceTextMix,
	Tag,
	GlobalVariable,
	Logic,
};

class InkObject {
protected:
	std::vector<struct ExpressionParser::Token*> function_return_values;

public:
	virtual ~InkObject();

	virtual std::string to_string() const;
	virtual std::vector<std::uint8_t> to_bytes() const;
	virtual InkObject* populate_from_bytes(const std::vector<std::uint8_t>& bytes, std::size_t& index);
	virtual ObjectId get_id() const = 0;
	virtual bool has_any_contents(bool strip) const { return true; }

	virtual void execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) = 0;

	virtual bool stop_before_this() const { return false; }
	
	ByteVec get_serialized_bytes() const;

	static InkObject* create_from_id(ObjectId id);

protected:
	bool prepare_next_function_call(struct ExpressionParser::ShuntedExpression& expression, InkStoryState& story_state, InkStoryEvalResult& eval_result,
									const ExpressionParser::VariableMap& variables, const ExpressionParser::VariableMap& constants, ExpressionParser::RedirectMap& redirects);
};

template <>
struct Serializer<InkObject*> {
	ByteVec operator()(const InkObject* value);
};

template <>
struct Deserializer<InkObject*> {
	InkObject* operator()(const ByteVec& bytes, std::size_t& index);
};

#include "runtime/ink_story_data.h"
