#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <cstring>

using ByteVec = std::vector<std::uint8_t>;

template <typename T>
struct Serializer {
	Serializer() = delete;
	Serializer(const Serializer& from) = delete;
	Serializer(Serializer&& from) = delete;
	Serializer& operator=(const Serializer& other) = delete;
	Serializer& operator=(Serializer&& other) = delete;
};

template <typename T>
struct Deserializer {
	Deserializer() = delete;
	Deserializer(const Deserializer& from) = delete;
	Deserializer(Deserializer&& from) = delete;
	Deserializer& operator=(const Deserializer& other) = delete;
	Deserializer& operator=(Deserializer&& other) = delete;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

template <>
struct Serializer<std::uint8_t> {
	ByteVec operator()(std::uint8_t value);
};

template <>
struct Deserializer<std::uint8_t> {
	std::uint8_t operator()(const ByteVec& bytes, std::size_t& index);
};

template <>
struct Serializer<std::uint16_t> {
	ByteVec operator()(std::uint16_t value);
};

template <>
struct Deserializer<std::uint16_t> {
	std::uint16_t operator()(const ByteVec& bytes, std::size_t& index);
};

template <>
struct Serializer<std::uint32_t> {
	ByteVec operator()(std::uint32_t value);
};

template <>
struct Deserializer<std::uint32_t> {
	std::uint32_t operator()(const ByteVec& bytes, std::size_t& index);
};

template <>
struct Serializer<std::uint64_t> {
	ByteVec operator()(std::uint64_t value);
};

template <>
struct Deserializer<std::uint64_t> {
	std::uint64_t operator()(const ByteVec& bytes, std::size_t& index);
};

template <>
struct Serializer<std::int64_t> {
	ByteVec operator()(std::int64_t value);
};

template <>
struct Deserializer<std::int64_t> {
	std::int64_t operator()(const ByteVec& bytes, std::size_t& index);
};

////////////////////////////////////////////////////////////////////////////////////////////////////

template <>
struct Serializer<float> {
	ByteVec operator()(float value);
};

template <>
struct Deserializer<float> {
	float operator()(const ByteVec& bytes, std::size_t& index);
};

template <>
struct Serializer<double> {
	ByteVec operator()(double value);
};

template <>
struct Deserializer<double> {
	double operator()(const ByteVec& bytes, std::size_t& index);
};

////////////////////////////////////////////////////////////////////////////////////////////////////

template <>
struct Serializer<std::string> {
	ByteVec operator()(const std::string& value);
};

template <>
struct Deserializer<std::string> {
	std::string operator()(const ByteVec& bytes, std::size_t& index);
};

////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
struct VectorSerializer {
	ByteVec operator()(const std::vector<T>& value) {
		Serializer<std::uint16_t> ssize;
		ByteVec result = ssize(static_cast<std::uint16_t>(value.size()));

		Serializer<T> s;
		for (const T& entry : value) {
			ByteVec bytes = s(entry);
			result.insert(result.end(), bytes.begin(), bytes.end());
		}

		return result;
	}
};

template <typename T>
struct VectorDeserializer {
	std::vector<T> operator()(const ByteVec& bytes, std::size_t& index) {
		std::vector<T> result;

		Deserializer<std::uint16_t> dssize;
		std::uint16_t size = dssize(bytes, index);
		result.reserve(size);

		Deserializer<T> ds;
		for (std::size_t i = 0; i < size; ++i) {
			result.push_back(ds(bytes, index));
		}

		//index += size;
		return result;
	}
};
