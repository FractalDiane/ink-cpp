#pragma once

#include <vector>
#include <string>

template <typename T>
struct Serializer {
	Serializer() = delete;
	Serializer(const Serializer& other) = delete;
	Serializer(Serializer&& other) = delete;
	Serializer& operator=(const Serializer& other) = delete;
	Serializer& operator=(Serializer&& other) = delete;
	
	std::vector<std::uint8_t> operator()(const T& what);
};

template <>
struct Serializer<std::uint8_t> {
	std::vector<std::uint8_t> operator()(const std::uint8_t& value) const noexcept {
		return {value};
	}
};

template <>
struct Serializer<std::uint16_t> {
	std::vector<std::uint8_t> operator()(const std::uint16_t& value) const noexcept {
		std::vector<std::uint8_t> result;
		result.push_back(value & 0xff);
		result.push_back((value & 0xff00) >> 8);
		return result;
	}
};

template <>
struct Serializer<std::uint32_t> {
	std::vector<std::uint8_t> operator()(const std::uint32_t& value) const noexcept {
		std::vector<std::uint8_t> result;
		result.push_back(value & 0xff);
		result.push_back((value & 0xff00) >> 8);
		result.push_back((value & 0xff0000) >> 16);
		result.push_back((value & 0xff000000) >> 24);
		return result;
	}
};

template <>
struct Serializer<float> {
	std::vector<std::uint8_t> operator()(const float& value) const noexcept {
		std::vector<std::uint8_t> result;
		std::uint32_t bytes;
		std::memcpy(&bytes, &value, sizeof(float));

		Serializer<std::uint32_t> s;
		return s(bytes);
	}
};

template <>
struct Serializer<std::string> {
	std::vector<std::uint8_t> operator()(const std::string& string) const noexcept {
		std::vector<std::uint8_t> result;
		std::uint16_t length = string.length();
		result.push_back(length & 0xff);
		result.push_back((length & 0xff00) >> 8);

		for (const char& byte : string) {
			result.push_back(byte);
		}

		return result;
	}
};
