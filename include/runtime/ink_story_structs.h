#pragma once

#include <string>
#include <vector>
#include <cstdint>

#include "serialization.h"

struct GatherPoint {
	std::string name;
	std::uint16_t index;
	std::uint8_t level;
};

struct Stitch {
	std::string name;
	std::uint16_t index;
	std::vector<GatherPoint> gather_points;
};

struct Knot {
	std::string name;
	std::vector<class InkObject*> objects;
	std::vector<Stitch> stitches;
	std::vector<GatherPoint> gather_points;
};

template <>
struct Serializer<Stitch> {
	std::vector<std::uint8_t> operator()(const Stitch& stitch) noexcept {
		Serializer<std::string> sstring;
		std::vector<std::uint8_t> result = sstring(stitch.name);
		Serializer<std::uint16_t> ssize;
		std::vector<std::uint8_t> result2 = ssize(stitch.index);
		result.insert(result.end(), result2.begin(), result2.end());

		return result;
	}

	Stitch operator()(const std::vector<std::uint8_t>& bytes, std::size_t& index) {
		Stitch result;
		Serializer<std::string> dsstring;
		result.name = dsstring(bytes, index);
		Serializer<std::uint16_t> dssize;
		result.index = dssize(bytes, index);
		
		return result;
	}
};

template <>
struct Serializer<Knot> {
	std::vector<std::uint8_t> operator()(const Knot& knot) noexcept {
		Serializer<std::string> sstring;
		std::vector<std::uint8_t> result = sstring(knot.name);

		Serializer<std::vector<InkObject*>> sobjects;
		std::vector<std::uint8_t> result2 = sobjects(knot.objects);
		result.insert(result.end(), result2.begin(), result2.end());

		Serializer<std::vector<Stitch>> sstitches;
		std::vector<std::uint8_t> result3 = sstitches(knot.stitches);
		result.insert(result.end(), result3.begin(), result3.end());

		return result;
	}
};
