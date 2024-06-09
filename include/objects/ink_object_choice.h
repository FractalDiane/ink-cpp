#pragma once

#include "objects/ink_object.h"

#include "expression_parser/expression_parser.h"

#include <string>
#include <unordered_set>
#include <optional>

struct InkChoiceEntry {
	std::vector<InkObject*> text;
	Knot result;
	bool sticky = false;
	std::vector<ExpressionParser::ShuntedExpression> conditions;
	bool fallback = false;
	bool immediately_continue_to_result = false;

	GatherPoint label;
};

class InkObjectChoice : public InkObject {
public:
	struct ChoiceComponents {
		std::string text;
		InkChoiceEntry* entry;
		std::size_t index = 0;
	};

	struct GetChoicesResult {
		std::vector<ChoiceComponents> choices;
		std::optional<std::size_t> fallback_index = 0;
		bool need_to_prepare_function = false;
	};

private:
	std::vector<InkChoiceEntry> choices;

	std::unordered_set<Uuid> conditions_fully_prepared;

public:
	InkObjectChoice(const std::vector<InkChoiceEntry>& choices) : choices{choices} {}
	virtual ~InkObjectChoice() override;

	virtual std::string to_string() const override;
	virtual ObjectId get_id() const override { return ObjectId::Choice; }

	virtual void execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) override;

	virtual ByteVec to_bytes() const override;
	virtual InkObject* populate_from_bytes(const ByteVec& bytes, std::size_t& index) override;

	GetChoicesResult get_choices(InkStoryState& story_state, InkStoryEvalResult& eval_result);

	virtual bool stop_before_this(const InkStoryState& story_state) const override { return story_state.choice_divert_index.has_value(); }
};

template <>
struct Serializer<InkChoiceEntry> {
	ByteVec operator()(const InkChoiceEntry& entry);
};

template <>
struct Deserializer<InkChoiceEntry> {
	InkChoiceEntry operator()(const ByteVec& bytes, std::size_t& index);
};
