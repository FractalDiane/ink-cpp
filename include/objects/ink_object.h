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
	List,
};

class InkObject {
public:
	typedef std::vector<struct ExpressionParserV2::ShuntedExpression*> ExpressionsVec;

public:
	virtual ~InkObject();

	virtual std::string to_string() const;
	virtual std::vector<std::uint8_t> to_bytes() const;
	virtual InkObject* populate_from_bytes(const std::vector<std::uint8_t>& bytes, std::size_t& index);
	virtual ObjectId get_id() const = 0;
	virtual bool has_any_contents(bool strip) const { return true; }
	virtual bool contributes_content_to_knot() const { return false; }

	virtual void execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) = 0;

	virtual bool stop_before_this(const InkStoryState& story_state) const { return false; }

	virtual ExpressionsVec get_all_expressions() { return {}; }
	
	ByteVec get_serialized_bytes() const;

	static InkObject* create_from_id(ObjectId id);

protected:
	ExpressionParserV2::ExecuteResult prepare_next_function_call(struct ExpressionParserV2::ShuntedExpression& expression, InkStoryState& story_state, InkStoryEvalResult& eval_result,
									ExpressionParserV2::StoryVariableInfo& story_variable_info);
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
