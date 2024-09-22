#pragma once

#include "objects/ink_object.h"

#include <string>
#include <unordered_set>

class InkObjectConditional : public InkObject {
public:
	using Entry = std::pair<struct ExpressionParserV2::ShuntedExpression, Knot>;
	
private:
	bool is_switch;
	std::vector<Entry> branches;
	Knot branch_else;

	ExpressionParserV2::ShuntedExpression switch_expression;

	std::unordered_set<Uuid> conditions_fully_prepared;

public:
	InkObjectConditional(const std::vector<std::pair<struct ExpressionParserV2::ShuntedExpression, Knot>>& branches, const Knot& objects_else)
		: branches{branches}, branch_else{objects_else}, is_switch{false} {}

	InkObjectConditional(const ExpressionParserV2::ShuntedExpression& switch_expression, const std::vector<std::pair<struct ExpressionParserV2::ShuntedExpression, Knot>>& branches, const Knot& objects_else)
		: switch_expression{switch_expression}, branches{branches}, branch_else{objects_else}, is_switch{true} {}
 
	virtual ~InkObjectConditional() override;

	virtual ObjectId get_id() const override { return ObjectId::Conditional; }

	virtual void execute(InkStoryState& stopstory_state, InkStoryEvalResult& eval_result) override;

	virtual bool contributes_content_to_knot() const override;

	virtual ExpressionsVec get_all_expressions() override;

	virtual ByteVec to_bytes() const override;
	InkObject* populate_from_bytes(const ByteVec& bytes, std::size_t& index);

	//virtual bool stop_before_this(const InkStoryState& story_state) const override { return true; }
};

template <>
struct Serializer<InkObjectConditional::Entry> {
	ByteVec operator()(const InkObjectConditional::Entry& entry);
};

template <>
struct Deserializer<InkObjectConditional::Entry> {
	InkObjectConditional::Entry operator()(const ByteVec& bytes, std::size_t& index);
};
