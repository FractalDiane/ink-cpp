#pragma once

#include "objects/ink_object.h"

#include "expression_parser/expression_parser.h"

#include <string>

class InkObjectDivert : public InkObject {
private:
	ExpressionParser::ShuntedExpression target_knot;
	std::vector<ExpressionParser::ShuntedExpression> arguments;
	DivertType type;

public:
	InkObjectDivert() : target_knot{}, arguments{}, type{DivertType::ToKnot} {}
	InkObjectDivert(const ExpressionParser::ShuntedExpression& target, const std::vector<ExpressionParser::ShuntedExpression>& arguments, DivertType type) : target_knot{target}, arguments{arguments}, type{type} {}
	
	virtual ~InkObjectDivert() override;

	virtual ObjectId get_id() const override { return ObjectId::Divert; }
	virtual ByteVec to_bytes() const override;
	virtual InkObject* populate_from_bytes(const ByteVec& bytes, std::size_t& index) override;
	//virtual std::string to_string() const override { return std::format("Divert ({})", target_knot); }
	virtual bool has_any_contents(bool strip) const override { return !target_knot.tokens.empty() || type == DivertType::FromTunnel; }

	virtual void execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) override;

	std::string get_target(InkStoryState& story_state, const ExpressionParser::VariableMap& story_constants);
};
