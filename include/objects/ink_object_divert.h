#pragma once

#include "objects/ink_object.h"

#include "expression_parser/expression_parser.h"

#include <string>
#include <format>

class InkObjectDivert : public InkObject {
private:
	std::vector<ExpressionParser::Token*> target_knot;
	std::vector<std::vector<ExpressionParser::Token*>> arguments;

public:
	InkObjectDivert() : target_knot{}, arguments{} {}
	InkObjectDivert(const std::vector<ExpressionParser::Token*>& target, const std::vector<std::vector<ExpressionParser::Token*>>& arguments) : target_knot{target}, arguments{arguments} {}
	
	virtual ~InkObjectDivert() override;

	virtual ObjectId get_id() const override { return ObjectId::Divert; }
	//virtual std::vector<std::uint8_t> to_bytes() const override;
	//virtual InkObject* populate_from_bytes(const std::vector<std::uint8_t>& bytes, std::size_t& index) override;
	//virtual std::string to_string() const override { return std::format("Divert ({})", target_knot); }
	virtual bool has_any_contents(bool strip) const override { return !target_knot.empty(); }

	virtual void execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) override;
};
