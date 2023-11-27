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
	
	std::vector<std::uint8_t> operator()(const T& what) noexcept;
	T operator()(const std::vector<std::uint8_t>& bytes, std::size_t& index) noexcept;
};

template <>
struct Serializer<std::uint8_t> {
	std::vector<std::uint8_t> operator()(const std::uint8_t& value) noexcept {
		return {value};
	}

	std::uint8_t operator()(const std::vector<std::uint8_t>& bytes, std::size_t& index) noexcept {
		return bytes[index++];
	}
};

template <>
struct Serializer<std::uint16_t> {
	std::vector<std::uint8_t> operator()(const std::uint16_t& value) noexcept {
		std::vector<std::uint8_t> result;
		result.push_back(value & 0xff);
		result.push_back((value & 0xff00) >> 8);
		return result;
	}

	std::uint16_t operator()(const std::vector<std::uint8_t>& bytes, std::size_t& index) noexcept {
		std::uint16_t result = 0;
		result |= static_cast<std::uint16_t>(bytes[index++]);
		result |= static_cast<std::uint16_t>(bytes[index++]) << 8;
		return result;
	}
};

template <>
struct Serializer<std::uint32_t> {
	std::vector<std::uint8_t> operator()(const std::uint32_t& value) noexcept {
		std::vector<std::uint8_t> result;
		result.push_back(value & 0xff);
		result.push_back((value & 0xff00) >> 8);
		result.push_back((value & 0xff0000) >> 16);
		result.push_back((value & 0xff000000) >> 24);
		return result;
	}

	std::uint32_t operator()(const std::vector<std::uint8_t>& bytes, std::size_t& index) noexcept {
		std::uint32_t result = 0;
		result |= static_cast<std::uint32_t>(bytes[index++]);
		result |= static_cast<std::uint32_t>(bytes[index++]) << 8;
		result |= static_cast<std::uint32_t>(bytes[index++]) << 16;
		result |= static_cast<std::uint32_t>(bytes[index++]) << 24;
		return result;
	}
};

template <>
struct Serializer<std::uint64_t> {
	std::vector<std::uint8_t> operator()(const std::uint64_t& value) noexcept {
		std::vector<std::uint8_t> result;
		result.push_back(value & 0xff);
		result.push_back((value & 0xff00) >> 8);
		result.push_back((value & 0xff0000) >> 16);
		result.push_back((value & 0xff000000) >> 24);
		result.push_back((value & 0xff00000000) >> 32);
		result.push_back((value & 0xff0000000000) >> 40);
		result.push_back((value & 0xff000000000000) >> 48);
		result.push_back((value & 0xff00000000000000) >> 56);
		return result;
	}

	std::uint64_t operator()(const std::vector<std::uint8_t>& bytes, std::size_t& index) noexcept {
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
};

template <>
struct Serializer<float> {
	std::vector<std::uint8_t> operator()(const float& value) noexcept {
		std::vector<std::uint8_t> result;
		std::uint32_t bytes;
		std::memcpy(&bytes, &value, sizeof(float));

		Serializer<std::uint32_t> s;
		return s(bytes);
	}

	float operator()(const std::vector<std::uint8_t>& bytes, std::size_t& index) noexcept {
		float result;

		Serializer<std::uint32_t> ds;
		std::uint32_t float_bytes = ds(bytes, index);
		std::memcpy(&result, &float_bytes, sizeof(float));
		return result;
	}
};

template <>
struct Serializer<std::string> {
	std::vector<std::uint8_t> operator()(const std::string& string) noexcept {
		std::vector<std::uint8_t> result;
		result.reserve(string.length() + 2);
		
		std::uint16_t length = string.length();
		result.push_back(length & 0xff);
		result.push_back((length & 0xff00) >> 8);

		for (const char& byte : string) {
			result.push_back(byte);
		}

		return result;
	}

	std::string operator()(const std::vector<std::uint8_t>& bytes, std::size_t& index) noexcept {
		std::string result;
		
		Serializer<std::uint16_t> ds;
		std::uint16_t length = ds(bytes, index);
		result.reserve(length + 1);

		for (std::size_t i = 0; i < length; ++i) {
			result += bytes[index + i];
		}

		index += length;
		return result;
	}
};

template <typename T>
struct Serializer<std::vector<T>> {
	std::vector<std::uint8_t> operator()(const std::vector<T>& vector) noexcept {
		Serializer<std::uint16_t> ssize;
		std::vector<std::uint8_t> result = ssize(static_cast<std::uint16_t>(vector.size()));
		
		for (const T& entry : vector) {
			Serializer<T> s;
			std::vector<std::uint8_t> bytes = s(entry);
			result.insert(result.end(), bytes.begin(), bytes.end());
		}

		return result;
	}

	std::vector<T> operator()(const std::vector<std::uint8_t>& bytes, std::size_t& index) noexcept {
		std::vector<T> result;

		Serializer<std::uint16_t> dssize;
		std::uint16_t size = dssize(bytes, index);
		result.reserve(size);

		Serializer<T> ds;
		for (std::size_t i = 0; i < size; ++i) {
			result.push_back(ds(bytes, index + i));
		}

		index += size;
		return result;
	}
};
