#include "serialization.h"

ByteVec Serializer<std::uint8_t>::operator()(std::uint8_t value) {
	return {value};
}


std::uint8_t Deserializer<std::uint8_t>::operator()(const ByteVec& bytes, std::size_t& index) {
	return bytes[index++];
}


ByteVec Serializer<std::uint16_t>::operator()(std::uint16_t value) {
	ByteVec result;
	result.push_back(static_cast<std::uint8_t>(value & 0xffu));
 	result.push_back(static_cast<std::uint8_t>((value & 0xff00u) >> 8));
	return result;
}


std::uint16_t Deserializer<std::uint16_t>::operator()(const ByteVec& bytes, std::size_t& index) {
	std::uint16_t result = 0;
	result |= static_cast<std::uint16_t>(bytes[index++]);
	result |= static_cast<std::uint16_t>(bytes[index++]) << 8;
	return result;
}


ByteVec Serializer<std::uint32_t>::operator()(std::uint32_t value) {
	ByteVec result;
	result.push_back(static_cast<std::uint8_t>(value & 0xff00u));
	result.push_back(static_cast<std::uint8_t>((value & 0xff00u) >> 8));
	result.push_back(static_cast<std::uint8_t>((value & 0xff0000u) >> 16));
	result.push_back(static_cast<std::uint8_t>((value & 0xff000000u) >> 24));
	return result;
}


std::uint32_t Deserializer<std::uint32_t>::operator()(const ByteVec& bytes, std::size_t& index) {
	std::uint32_t result = 0;
	result |= static_cast<std::uint32_t>(bytes[index++]);
	result |= static_cast<std::uint32_t>(bytes[index++]) << 8;
	result |= static_cast<std::uint32_t>(bytes[index++]) << 16;
	result |= static_cast<std::uint32_t>(bytes[index++]) << 24;
	return result;
}


ByteVec Serializer<std::uint64_t>::operator()(std::uint64_t value) {
	ByteVec result;
	result.push_back(static_cast<std::uint8_t>(value & 0xffu));
	result.push_back(static_cast<std::uint8_t>((value & 0xff00u) >> 8));
	result.push_back(static_cast<std::uint8_t>((value & 0xff0000u) >> 16));
	result.push_back(static_cast<std::uint8_t>((value & 0xff000000u) >> 24));
	result.push_back(static_cast<std::uint8_t>((value & 0xff00000000u) >> 32));
	result.push_back(static_cast<std::uint8_t>((value & 0xff0000000000u) >> 40));
	result.push_back(static_cast<std::uint8_t>((value & 0xff000000000000u) >> 48));
	result.push_back(static_cast<std::uint8_t>((value & 0xff00000000000000u) >> 56));
	return result;
}


std::uint64_t Deserializer<std::uint64_t>::operator()(const ByteVec& bytes, std::size_t& index) {
	std::uint64_t result = 0;
	result |= static_cast<std::uint64_t>(bytes[index++]);
	result |= static_cast<std::uint64_t>(bytes[index++]) << 8;
	result |= static_cast<std::uint64_t>(bytes[index++]) << 16;
	result |= static_cast<std::uint64_t>(bytes[index++]) << 24;
	result |= static_cast<std::uint64_t>(bytes[index++]) << 32;
	result |= static_cast<std::uint64_t>(bytes[index++]) << 40;
	result |= static_cast<std::uint64_t>(bytes[index++]) << 48;
	result |= static_cast<std::uint64_t>(bytes[index++]) << 56;
	return result;
}

ByteVec Serializer<std::int64_t>::operator()(std::int64_t value) {
	ByteVec result;
	result.push_back(static_cast<std::uint8_t>(value & 0xffu));
	result.push_back(static_cast<std::uint8_t>((value & 0xff00u) >> 8));
	result.push_back(static_cast<std::uint8_t>((value & 0xff0000u) >> 16));
	result.push_back(static_cast<std::uint8_t>((value & 0xff000000u) >> 24));
	result.push_back(static_cast<std::uint8_t>((value & 0xff00000000u) >> 32));
	result.push_back(static_cast<std::uint8_t>((value & 0xff0000000000u) >> 40));
	result.push_back(static_cast<std::uint8_t>((value & 0xff000000000000u) >> 48));
	result.push_back(static_cast<std::uint8_t>((value & 0xff00000000000000u) >> 56));
	return result;
}


std::int64_t Deserializer<std::int64_t>::operator()(const ByteVec& bytes, std::size_t& index) {
	std::int64_t result = 0;
	result |= static_cast<std::int64_t>(bytes[index++]);
	result |= static_cast<std::int64_t>(bytes[index++]) << 8;
	result |= static_cast<std::int64_t>(bytes[index++]) << 16;
	result |= static_cast<std::int64_t>(bytes[index++]) << 24;
	result |= static_cast<std::int64_t>(bytes[index++]) << 32;
	result |= static_cast<std::int64_t>(bytes[index++]) << 40;
	result |= static_cast<std::int64_t>(bytes[index++]) << 48;
	result |= static_cast<std::int64_t>(bytes[index++]) << 56;
	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////


ByteVec Serializer<float>::operator()(float value) {
	ByteVec result;
	std::uint32_t bytes;
	std::memcpy(&bytes, &value, sizeof(float));

	Serializer<std::uint32_t> s;
	return s(bytes);
}


float Deserializer<float>::operator()(const ByteVec& bytes, std::size_t& index) {
	float result;

	Deserializer<std::uint32_t> ds;
	std::uint32_t float_bytes = ds(bytes, index);
	std::memcpy(&result, &float_bytes, sizeof(float));
	return result;
}


ByteVec Serializer<double>::operator()(double value) {
	ByteVec result;
	std::uint64_t bytes;
	std::memcpy(&bytes, &value, sizeof(double));

	Serializer<std::uint64_t> s;
	return s(bytes);
}


double Deserializer<double>::operator()(const ByteVec& bytes, std::size_t& index) {
	double result;

	Deserializer<std::uint64_t> ds;
	std::uint64_t double_bytes = ds(bytes, index);
	std::memcpy(&result, &double_bytes, sizeof(double));
	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////


ByteVec Serializer<std::string>::operator()(const std::string& value) {
	ByteVec result;
	result.reserve(value.length() + 2);
	
	std::uint16_t length = static_cast<std::uint16_t>(value.length());
	result.push_back(length & 0xffu);
	result.push_back((length & 0xff00u) >> 8);

	for (const char& byte : value) {
		result.push_back(static_cast<std::uint8_t>(byte));
	}

	return result;
}


std::string Deserializer<std::string>::operator()(const ByteVec& bytes, std::size_t& index) {
	std::string result;
	
	Deserializer<std::uint16_t> ds;
	std::uint16_t length = ds(bytes, index);
	result.reserve(length + 1u);

	for (std::size_t i = 0; i < length; ++i) {
		result += static_cast<signed char>(bytes[index + i]);
	}

	index += length;
	return result;
}
