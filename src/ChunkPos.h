#pragma once

#include <unordered_map>
#include <cstdint>

struct ChunkPos {
	int64_t x, y, z;
	bool operator==(const ChunkPos &other) const {
		return x == other.x && y == other.y && z == other.z;
	}
};

namespace std {
	template <>
	struct hash<ChunkPos> {
		std::size_t operator()(const ChunkPos& pos) const {
			const int64_t dimensions[] { pos.x, pos.y, pos.z };

			constexpr uint8_t NUM_DIMENSIONS = 3;
			constexpr uint16_t NUM_HEX_CHARS = NUM_DIMENSIONS * (2 * sizeof(int64_t));
			constexpr char HEX[] {
					'0', '1', '2', '3',
					'4', '5', '6', '7',
					'8', '9', 'A', 'B',
					'C', 'D', 'E', 'F'};

			uint32_t hexIndex = 0;
			char hexBuffer[NUM_HEX_CHARS];
			for(int dim = 0; dim < NUM_DIMENSIONS; dim++) {
				const int &value = dimensions[dim];
				for(int byte = 0; byte < sizeof(int64_t); byte++) {
					const uint8_t byteValue = (value >> (byte * 8)) & 0xFF;
					hexBuffer[hexIndex++] = HEX[byteValue & 0x0F];
					hexBuffer[hexIndex++] = HEX[byteValue >> 4];
				}
			}

			return std::_Hash_impl::hash(hexBuffer, NUM_HEX_CHARS);
		}
	};
}