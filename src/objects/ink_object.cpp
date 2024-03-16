#include "objects/ink_object.h"

ByteVec Serializer<InkObject*>::operator()(const InkObject* value) {
	return value->get_serialized_bytes();
}

InkObject* Deserializer<InkObject*>::operator()(const ByteVec& bytes, std::size_t& index) {
	Deserializer<std::uint8_t> ds8;

	ObjectId id = static_cast<ObjectId>(ds8(bytes, index));
	InkObject* result = InkObject::create_from_id(id);
	return result->populate_from_bytes(bytes, index);
}

std::string InkObject::to_string() const {
	return "NO TO_STRING";
}

std::vector<std::uint8_t> InkObject::to_bytes() const {
	return {};
}

InkObject* InkObject::populate_from_bytes(const std::vector<std::uint8_t>& bytes, std::size_t& index) {
	return this;
}

ByteVec InkObject::get_serialized_bytes() const {
	ByteVec result = {static_cast<std::uint8_t>(get_id())};
	ByteVec serialization = to_bytes();
	result.insert(result.end(), serialization.begin(), serialization.end());
	return result;
}

#include "objects/ink_object_choice.h"
#include "objects/ink_object_choicetextmix.h"
#include "objects/ink_object_conditional.h"
#include "objects/ink_object_divert.h"
#include "objects/ink_object_globalvariable.h"
#include "objects/ink_object_glue.h"
#include "objects/ink_object_interpolation.h"
#include "objects/ink_object_linebreak.h"
#include "objects/ink_object_logic.h"
#include "objects/ink_object_sequence.h"
#include "objects/ink_object_tag.h"
#include "objects/ink_object_text.h"

InkObject* InkObject::create_from_id(ObjectId id) {
	switch (id) {
		case ObjectId::Text: {
			return new InkObjectText();
		} break;

		case ObjectId::Choice: {
			return new InkObjectChoice({});
		} break;

		case ObjectId::LineBreak: {
			return new InkObjectLineBreak();
		} break;

		case ObjectId::Glue: {
			return new InkObjectGlue();
		} break;

		case ObjectId::Divert: {
			return new InkObjectDivert({}, {});
		} break;

		case ObjectId::Interpolation: {
			return new InkObjectInterpolation({});
		} break;

		case ObjectId::Conditional: {
			return new InkObjectConditional({}, {});
		} break;

		case ObjectId::Sequence: {
			return new InkObjectSequence(InkSequenceType::Sequence, false, {});
		} break;

		case ObjectId::ChoiceTextMix: {
			return new InkObjectChoiceTextMix(false);
		} break;

		case ObjectId::Tag: {
			return new InkObjectTag("");
		} break;

		case ObjectId::GlobalVariable: {
			return new InkObjectGlobalVariable("", false, {});
		} break;

		case ObjectId::Logic: {
			return new InkObjectLogic({});
		} break;

		default: {
			throw std::runtime_error(std::format("Tried to create an inkb object with an unknown object ID ({})", static_cast<std::uint8_t>(id)));
		} break;
	}
}
