#include "runtime/ink_story_structs.h"

#include "objects/ink_object.h"

ByteVec Serializer<InkWeaveContent::Parameter>::operator()(const InkWeaveContent::Parameter& parameter) {
	Serializer<std::uint8_t> s8;
	Serializer<std::string> sstring;

	ByteVec result = sstring(parameter.name);
	ByteVec result2 = s8(static_cast<std::uint8_t>(parameter.by_ref));
	result.append_range(result2);

	return result;
}

InkWeaveContent::Parameter Deserializer<InkWeaveContent::Parameter>::operator()(const ByteVec& bytes, std::size_t& index) {
	Deserializer<std::uint8_t> ds8;
	Deserializer<std::string> dsstring;
	
	InkWeaveContent::Parameter result;
	result.name = dsstring(bytes, index);
	result.by_ref = static_cast<bool>(ds8(bytes, index));
	return result;
}

ByteVec Serializer<GatherPoint>::operator()(const GatherPoint& gather_point) {
	Serializer<std::uint8_t> s8;
	Serializer<std::uint16_t> s16;
	Serializer<std::string> sstring;
	
	ByteVec result = sstring(gather_point.name);

	ByteVec result3 = s16(gather_point.index);
	ByteVec result4 = s8(gather_point.level);
	ByteVec result5 = s8(static_cast<std::uint8_t>(gather_point.in_choice));
	ByteVec result6 = s16(gather_point.choice_index);

	result.append_range(result3);
	result.append_range(result4);
	result.append_range(result5);
	result.append_range(result6);

	return result;
}

GatherPoint Deserializer<GatherPoint>::operator()(const ByteVec& bytes, std::size_t& index) {
	Deserializer<std::uint8_t> ds8;
	Deserializer<std::uint16_t> ds16;
	Deserializer<std::string> dsstring;

	GatherPoint result;
	result.name = dsstring(bytes, index);

	result.index = ds16(bytes, index);
	result.level = ds8(bytes, index);
	result.in_choice = static_cast<bool>(ds8(bytes, index));
	result.choice_index = ds16(bytes, index);

	result.uuid = 0;
	result.type = WeaveContentType::GatherPoint;

	return result;
}

ByteVec Serializer<Stitch>::operator()(const Stitch& stitch) {
	//Serializer<std::uint8_t> s8;
	Serializer<std::uint16_t> s16;
	Serializer<std::string> sstring;
	VectorSerializer<InkWeaveContent::Parameter> vsparam;
	VectorSerializer<GatherPoint> vsgatherpoint;

	ByteVec result = sstring(stitch.name);
	ByteVec result2 = vsparam(stitch.parameters);

	ByteVec result3 = s16(stitch.index);
	ByteVec result4 = vsgatherpoint(stitch.gather_points);

	result.append_range(result2);
	result.append_range(result3);
	result.append_range(result4);
	
	return result;
}

Stitch Deserializer<Stitch>::operator()(const ByteVec& bytes, std::size_t& index) {
	//Deserializer<std::uint8_t> ds8;
	Deserializer<std::uint16_t> ds16;
	Deserializer<std::string> dsstring;
	VectorDeserializer<InkWeaveContent::Parameter> vdsparam;
	VectorDeserializer<GatherPoint> vdsgatherpoint;

	Stitch result;
	result.name = dsstring(bytes, index);
	result.parameters = vdsparam(bytes, index);

	result.index = ds16(bytes, index);
	result.gather_points = vdsgatherpoint(bytes, index);

	result.uuid = 0;
	result.type = WeaveContentType::Stitch;

	return result;
}

ByteVec Serializer<Knot>::operator()(const Knot& knot) {
	Serializer<std::uint8_t> s8;
	//Serializer<std::uint16_t> s16;
	Serializer<std::string> sstring;
	VectorSerializer<InkWeaveContent::Parameter> vsparam;
	VectorSerializer<InkObject*> vsobject;
	VectorSerializer<Stitch> vsstitch;
	VectorSerializer<GatherPoint> vsgatherpoint;
	
	ByteVec result = sstring(knot.name);
	ByteVec result2 = vsparam(knot.parameters);

	ByteVec result3 = vsstitch(knot.stitches);
	ByteVec result4 = vsgatherpoint(knot.gather_points);
	ByteVec result5 = s8(static_cast<std::uint8_t>(knot.is_function));
	ByteVec result6 = vsobject(knot.objects);

	result.append_range(result2);
	result.append_range(result3);
	result.append_range(result4);
	result.append_range(result5);
	result.append_range(result6);

	return result;
}

Knot Deserializer<Knot>::operator()(const ByteVec& bytes, std::size_t& index) {
	Deserializer<std::uint8_t> ds8;
	//Deserializer<std::uint16_t> ds16;
	Deserializer<std::string> dsstring;
	VectorDeserializer<InkWeaveContent::Parameter> vdsparam;
	//VectorDeserializer<InkObject*> vdsobject;
	VectorDeserializer<Stitch> vdsstitch;
	VectorDeserializer<GatherPoint> vdsgatherpoint;

	Knot result;
	result.name = dsstring(bytes, index);
	result.parameters = vdsparam(bytes, index);
	
	result.stitches = vdsstitch(bytes, index);
	result.gather_points = vdsgatherpoint(bytes, index);
	result.is_function = static_cast<bool>(ds8(bytes, index));
	//result.objects = vdsobject(bytes, index);

	result.uuid = 0;
	result.type = WeaveContentType::Knot;

	return result;
}
