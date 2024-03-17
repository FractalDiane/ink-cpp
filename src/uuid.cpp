#include "uuid.h"

ByteVec Serializer<Uuid>::operator()(const Uuid& uuid) {
	ByteVec result;
	std::uint32_t value = uuid.get();

	while (value >= 128) {
		result.push_back(static_cast<std::uint8_t>(value & 0x7f));
		value >>= 7;
	}

	result.push_back(static_cast<std::uint8_t>(value | 0x80));
	return result;
}

Uuid Deserializer<Uuid>::operator()(const ByteVec& bytes, std::size_t& index) {
	Deserializer<std::uint8_t> ds8;
	UuidValue result = 0;

	for (int i = 0; i < 5; ++i) {
		std::uint8_t byte = ds8(bytes, index);
		result |= (byte & 0x7f) << 7 * i;
		if ((byte & 0x80) != 0) {
			break;
		}
	}

	return Uuid(result);
}
