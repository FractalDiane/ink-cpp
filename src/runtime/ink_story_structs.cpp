#include "runtime/ink_story_structs.h"

#include "objects/ink_object.h"

#include <format>

ByteVec Serializer<InkWeaveContent::Parameter>::operator()(const InkWeaveContent::Parameter& parameter) {
	Serializer<std::uint8_t> s8;
	Serializer<std::string> sstring;

	ByteVec result = sstring(parameter.name);
	ByteVec result2 = s8(static_cast<std::uint8_t>(parameter.by_ref));
	result.insert(result.end(), result2.begin(), result2.end());

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
	Serializer<Uuid> suuid;
	
	ByteVec result = sstring(gather_point.name);
	ByteVec result2 = suuid(gather_point.uuid);

	ByteVec result3 = s16(gather_point.index);
	ByteVec result4 = s8(gather_point.level);
	ByteVec result5 = s8(static_cast<std::uint8_t>(gather_point.in_choice));
	ByteVec result6 = s16(gather_point.choice_index);

	result.insert(result.end(), result2.begin(), result2.end());
	result.insert(result.end(), result3.begin(), result3.end());
	result.insert(result.end(), result4.begin(), result4.end());
	result.insert(result.end(), result5.begin(), result5.end());
	result.insert(result.end(), result6.begin(), result6.end());

	return result;
}

GatherPoint Deserializer<GatherPoint>::operator()(const ByteVec& bytes, std::size_t& index) {
	Deserializer<std::uint8_t> ds8;
	Deserializer<std::uint16_t> ds16;
	Deserializer<std::string> dsstring;
	Deserializer<Uuid> dsuuid;

	GatherPoint result;
	result.name = dsstring(bytes, index);
	result.uuid = dsuuid(bytes, index);

	result.index = ds16(bytes, index);
	result.level = ds8(bytes, index);
	result.in_choice = static_cast<bool>(ds8(bytes, index));
	result.choice_index = ds16(bytes, index);

	result.type = WeaveContentType::GatherPoint;

	return result;
}

ByteVec Serializer<Stitch>::operator()(const Stitch& stitch) {
	//Serializer<std::uint8_t> s8;
	Serializer<std::uint16_t> s16;
	Serializer<std::string> sstring;
	VectorSerializer<InkWeaveContent::Parameter> vsparam;
	VectorSerializer<GatherPoint> vsgatherpoint;
	Serializer<Uuid> suuid;

	ByteVec result = sstring(stitch.name);
	ByteVec result2 = suuid(stitch.uuid);
	ByteVec result3 = vsparam(stitch.parameters);

	ByteVec result4 = s16(stitch.index);
	ByteVec result5 = vsgatherpoint(stitch.gather_points);

	result.insert(result.end(), result2.begin(), result2.end());
	result.insert(result.end(), result3.begin(), result3.end());
	result.insert(result.end(), result4.begin(), result4.end());
	result.insert(result.end(), result5.begin(), result5.end());
	
	return result;
}

Stitch Deserializer<Stitch>::operator()(const ByteVec& bytes, std::size_t& index) {
	//Deserializer<std::uint8_t> ds8;
	Deserializer<std::uint16_t> ds16;
	Deserializer<std::string> dsstring;
	VectorDeserializer<InkWeaveContent::Parameter> vdsparam;
	VectorDeserializer<GatherPoint> vdsgatherpoint;
	Deserializer<Uuid> dsuuid;

	Stitch result;
	result.name = dsstring(bytes, index);
	result.uuid = dsuuid(bytes, index);
	result.parameters = vdsparam(bytes, index);

	result.index = ds16(bytes, index);
	result.gather_points = vdsgatherpoint(bytes, index);

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
	Serializer<Uuid> suuid;
	
	ByteVec result = sstring(knot.name);
	ByteVec result2 = suuid(knot.uuid);
	ByteVec result3 = vsparam(knot.parameters);

	ByteVec result4 = vsstitch(knot.stitches);
	ByteVec result5 = vsgatherpoint(knot.gather_points);
	ByteVec result6 = s8(static_cast<std::uint8_t>(knot.is_function));
	ByteVec result7 = s8(static_cast<std::uint8_t>(knot.is_choice_result));
	ByteVec result8 = vsobject(knot.objects);

	result.insert(result.end(), result2.begin(), result2.end());
	result.insert(result.end(), result3.begin(), result3.end());
	result.insert(result.end(), result4.begin(), result4.end());
	result.insert(result.end(), result5.begin(), result5.end());
	result.insert(result.end(), result6.begin(), result6.end());
	result.insert(result.end(), result7.begin(), result7.end());
	result.insert(result.end(), result8.begin(), result8.end());

	return result;
}

Knot Deserializer<Knot>::operator()(const ByteVec& bytes, std::size_t& index) {
	Deserializer<std::uint8_t> ds8;
	//Deserializer<std::uint16_t> ds16;
	Deserializer<std::string> dsstring;
	VectorDeserializer<InkWeaveContent::Parameter> vdsparam;
	VectorDeserializer<InkObject*> vdsobject;
	VectorDeserializer<Stitch> vdsstitch;
	VectorDeserializer<GatherPoint> vdsgatherpoint;
	Deserializer<Uuid> dsuuid;

	Knot result;
	result.name = dsstring(bytes, index);
	result.uuid = dsuuid(bytes, index);
	result.parameters = vdsparam(bytes, index);
	
	result.stitches = vdsstitch(bytes, index);
	result.gather_points = vdsgatherpoint(bytes, index);
	result.is_function = static_cast<bool>(ds8(bytes, index));
	result.is_choice_result = static_cast<bool>(ds8(bytes, index));
	result.objects = vdsobject(bytes, index);

	result.type = WeaveContentType::Knot;

	return result;
}

std::string Knot::divert_target_to_global(const std::string& target) const {
	for (const Stitch& stitch : stitches) {
		if (stitch.name == target) {
			return std::format("{}.{}", name, stitch.name);
		}

		for (const GatherPoint& gather_point : stitch.gather_points) {
			if (gather_point.name == target || (target.contains(gather_point.name) && target.contains(stitch.name))) {
				return std::format("{}.{}.{}", name, stitch.name, gather_point.name);
			}
		}
	}

	for (const GatherPoint& gather_point : gather_points) {
		if (gather_point.name == target) {
			return std::format("{}.{}", name, gather_point.name);
		}
	}

	return target;
}

std::vector<GatherPoint*> Knot::get_all_gather_points() {
	std::vector<GatherPoint*> result;
	for (GatherPoint& gather_point : gather_points) {
		result.push_back(&gather_point);
	}

	for (Stitch& stitch : stitches) {
		for (GatherPoint& gather_point : stitch.gather_points) {
			result.push_back(&gather_point);
		}
	}

	return result;
}

void Knot::append_knot(const Knot& other) {
	for (InkObject* object : other.objects) {
		objects.push_back(object);
	}

	for (const Stitch& stitch : other.stitches) {
		stitches.push_back(stitch);
	}

	for (const GatherPoint& gather_point : other.gather_points) {
		gather_points.push_back(gather_point);
	}
}
