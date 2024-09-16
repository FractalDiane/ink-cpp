#pragma once

#include "objects/ink_object.h"

#include "expression_parser/expression_parser.h"

#include <string>
#include <unordered_set>
#include <optional>

struct InkChoiceEntry {
	std::vector<InkObject*> text;
	std::size_t index;
	Knot result;
	bool sticky = false;
	std::vector<ExpressionParserV2::ShuntedExpression> conditions;
	bool fallback = false;
	bool immediately_continue_to_result = false;

	GatherPoint label;

	InkChoiceEntry() { result.is_choice_result = true; }
	InkChoiceEntry(std::size_t index, bool sticky) : index(index), sticky(sticky) {
		result.is_choice_result = true;
	}
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
		FunctionPrepType function_prep_type = FunctionPrepType::None;
	};

private:
	std::vector<InkChoiceEntry> choices;

	std::unordered_set<Uuid> conditions_fully_prepared;
	std::unordered_set<InkObject*> text_objects_being_prepared;
	std::unordered_map<InkObject*, std::string> text_objects_fully_prepared;

public:
	InkObjectChoice(const std::vector<InkChoiceEntry>& choices) : choices{choices} {}
	virtual ~InkObjectChoice() override;

	virtual std::string to_string() const override;
	virtual ObjectId get_id() const override { return ObjectId::Choice; }

	virtual bool contributes_content_to_knot() const override { return true; }

	virtual void execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) override;

	virtual ByteVec to_bytes() const override;
	virtual InkObject* populate_from_bytes(const ByteVec& bytes, std::size_t& index) override;

	GetChoicesResult get_choices(InkStoryState& story_state, InkStoryEvalResult& eval_result);
	//std::vector<std::pair<std::size_t, GatherPoint*>> get_choice_labels();
	std::vector<ChoiceLabelData> get_choice_labels(Knot* containing_knot, std::size_t index_in_knot);

	virtual bool stop_before_this(const InkStoryState& story_state) const override { return story_state.choice_divert_index.has_value(); }

	virtual ExpressionsVec get_all_expressions() override;

private:
	static void populate_choice_labels_recursive(InkObjectChoice* object, Knot* containing_knot, std::size_t index_in_knot, std::vector<ChoiceLabelData>& all_labels);
};

template <>
struct Serializer<InkChoiceEntry> {
	ByteVec operator()(const InkChoiceEntry& entry);
};

template <>
struct Deserializer<InkChoiceEntry> {
	InkChoiceEntry operator()(const ByteVec& bytes, std::size_t& index);
};
