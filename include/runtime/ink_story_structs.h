#pragma once

#include <string>
#include <vector>
#include <cstdint>

#include "serialization.h"
#include "uuid.h"

enum class WeaveContentType {
	Knot,
	Stitch,
	GatherPoint,
};

enum class DivertType {
	ToKnot,

	ToTunnel,
	FromTunnel,

	Thread,
};

//typedef std::uint32_t Uuid;

struct InkWeaveContent {
	std::string name;
	Uuid uuid;
	WeaveContentType type;

	struct Parameter {
		std::string name;
		bool by_ref;
	};

	std::vector<Parameter> parameters;
};

struct GatherPoint : public InkWeaveContent {
	std::uint16_t index = 0;
	std::uint8_t level = 1;
	bool in_choice = false;
	std::uint16_t choice_index = 0;
};

struct Stitch : public InkWeaveContent {
	std::uint16_t index = 0;
	std::vector<GatherPoint> gather_points;
};

struct Knot : public InkWeaveContent {
	std::vector<class InkObject*> objects;
	std::vector<Stitch> stitches;
	std::vector<GatherPoint> gather_points;
	bool is_function = false;

	Knot() : objects{}, stitches{}, gather_points{} {}
	Knot(const std::vector<class InkObject*> objects) : objects{objects}, stitches{{}}, gather_points{{}} {}
};

template <>
struct Serializer<InkWeaveContent::Parameter> {
	ByteVec operator()(const InkWeaveContent::Parameter& parameter);
};

template <>
struct Deserializer<InkWeaveContent::Parameter> {
	InkWeaveContent::Parameter operator()(const ByteVec& bytes, std::size_t& index);
};

template <>
struct Serializer<GatherPoint> {
	ByteVec operator()(const GatherPoint& gather_point);
};

template <>
struct Deserializer<GatherPoint> {
	GatherPoint operator()(const ByteVec& bytes, std::size_t& index);
};

template <>
struct Serializer<Stitch> {
	ByteVec operator()(const Stitch& stitch);
};

template <>
struct Deserializer<Stitch> {
	Stitch operator()(const ByteVec& bytes, std::size_t& index);
};

template <>
struct Serializer<Knot> {
	ByteVec operator()(const Knot& knot);
};

template <>
struct Deserializer<Knot> {
	Knot operator()(const ByteVec& bytes, std::size_t& index);
};
