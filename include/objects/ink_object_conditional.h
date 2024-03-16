#pragma once

#include "objects/ink_object.h"

#include <string>

class InkObjectConditional : public InkObject {
public:
	using Entry = std::pair<std::vector<struct ExpressionParser::Token*>, Knot>;
	
private:
	bool is_switch;
	std::vector<Entry> branches;
	Knot branch_else;

	std::vector<struct ExpressionParser::Token*> switch_expression;

public:
	InkObjectConditional(const std::vector<std::pair<std::vector<struct ExpressionParser::Token*>, Knot>>& branches, const Knot& objects_else)
		: branches{branches}, branch_else{objects_else}, is_switch{false} {}

	InkObjectConditional(const std::vector<struct ExpressionParser::Token*>& switch_expression, const std::vector<std::pair<std::vector<struct ExpressionParser::Token*>, Knot>>& branches, const Knot& objects_else)
		: switch_expression{switch_expression}, branches{branches}, branch_else{objects_else}, is_switch{true} {}
 
	virtual ~InkObjectConditional() override;

	virtual ObjectId get_id() const override { return ObjectId::Conditional; }

	virtual void execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) override;

	virtual ByteVec to_bytes() const override;
	InkObject* populate_from_bytes(const ByteVec& bytes, std::size_t& index);
};

template <>
struct Serializer<InkObjectConditional::Entry> {
	ByteVec operator()(const InkObjectConditional::Entry& entry);
};

template <>
struct Deserializer<InkObjectConditional::Entry> {
	InkObjectConditional::Entry operator()(const ByteVec& bytes, std::size_t& index);
};
