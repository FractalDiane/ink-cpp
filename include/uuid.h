#pragma once

#include <cstdint>
#include <memory>

#include "serialization.h"

typedef std::uint32_t UuidValue;
class Uuid {
private:
	UuidValue uuid;

public:
	Uuid() : uuid{0} {}
	Uuid(UuidValue value) : uuid{value} {}

	UuidValue get() const { return uuid; }

	bool operator==(const Uuid& other) const {
		return uuid == other.uuid;
	}

	bool operator!=(const Uuid& other) const {
		return uuid != other.uuid;
	}
};

template <>
struct std::hash<Uuid> {
	std::size_t operator()(const Uuid& uuid) const noexcept {
		std::hash<std::uint32_t> h;
		return h(uuid.get());
	}
};

template <>
struct Serializer<Uuid> {
	ByteVec operator()(const Uuid& uuid);
};

template <>
struct Deserializer<Uuid> {
	Uuid operator()(const ByteVec& bytes, std::size_t& index);
};
