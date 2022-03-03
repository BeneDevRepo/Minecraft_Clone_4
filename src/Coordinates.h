#pragma once

#include <unordered_map>
#include <cstdint>
#include <glm/glm.hpp>







/****************************************
 * --------   CHUNK POSITION   -------- *
 ****************************************/

class ChunkPos {
private:
	int64_t data[3];

public:
	inline ChunkPos(): data { } { }
	inline ChunkPos(const int64_t x, const int64_t y, const int64_t z): data { x, y, z } { }

public:
	inline bool operator==(const ChunkPos &other) const {
		return data[0] == other.data[0] && data[1] == other.data[1] && data[2] == other.data[2];
	}

public:
	inline int64_t &operator[](const uint8_t dimension) { return data[dimension]; }
	inline int64_t operator[](const uint8_t dimension) const { return data[dimension]; }
	inline int64_t &x() { return data[0]; }
	inline int64_t x() const { return data[0]; }
	inline int64_t &y() { return data[1]; }
	inline int64_t y() const { return data[1]; }
	inline int64_t &z() { return data[2]; }
	inline int64_t z() const { return data[2]; }
	inline glm::vec3 toVec3() const { return glm::vec3{ x(), y(), z() }; }

public:
	inline ChunkPos operator+(const ChunkPos &other) const { return ChunkPos{ x()+other.x(), y()+other.y(), z()+other.z() }; }
	inline ChunkPos operator-(const ChunkPos &other) const { return ChunkPos{ x()-other.x(), y()-other.y(), z()-other.z() }; }
	inline ChunkPos &operator+=(const ChunkPos &other) {
		for(int dim = 0; dim < 3; dim++) data[dim] += other.data[dim];
		return *this;
	}
	inline ChunkPos &operator-=(const ChunkPos &other) {
		for(int dim = 0; dim < 3; dim++) data[dim] -= other.data[dim];
		return *this;
	}
};

namespace std {
	template <>
	struct hash<ChunkPos> {
		std::size_t operator()(const ChunkPos& pos) const {
			// return std::hash<std::string>{}(std::to_string(pos.x()) + std::to_string(pos.y()) + std::to_string(pos.z()));

			return std::_Hash_impl::hash(&pos, sizeof(ChunkPos));

			// constexpr uint8_t NUM_DIMENSIONS = 3;
			// constexpr uint8_t NUM_HEX_CHARS = NUM_DIMENSIONS * (2 * sizeof(int64_t));
			// constexpr char HEX[] {
			// 		'0', '1', '2', '3',
			// 		'4', '5', '6', '7',
			// 		'8', '9', 'A', 'B',
			// 		'C', 'D', 'E', 'F'};

			// uint8_t hexIndex = 0;
			// static thread_local char hexBuffer[NUM_HEX_CHARS];
			// for(int dim = 0; dim < NUM_DIMENSIONS; dim++) {
			// 	const int &value = pos[dim];
			// 	for(int byte = 0; byte < sizeof(int64_t); byte++) {
			// 		const uint8_t byteValue = (value >> (byte * 8)) & 0xFF;
			// 		hexBuffer[hexIndex++] = HEX[byteValue & 0x0F];
			// 		hexBuffer[hexIndex++] = HEX[byteValue >> 4];
			// 	}
			// }

			// return std::_Hash_impl::hash(hexBuffer, NUM_HEX_CHARS);
		}
	};
}










/****************************************
 * --------   BLOCK POSITION   -------- *
 ****************************************/

class BlockPos {
private:
	ChunkPos m_ChunkPos; // Coordinates of containing Chunk
	glm::ivec3 m_BlockPos; // Coordinates relative to chunk

public:
	// inline BlockPos(const glm::vec3 &absolutePos):
	// 		m_ChunkPos{
	// 			(int64_t)std::floor(absolutePos.x / 16.f),
	// 			(int64_t)std::floor(absolutePos.y / 16.f),
	// 			(int64_t)std::floor(absolutePos.z / 16.f),
	// 		},
	// 		m_RelativePos(absolutePos - glm::floor(absolutePos / 16.f) * 16.f) {
	// 	}
	inline BlockPos(const ChunkPos &chunkPos, const glm::ivec3 &blockPos):
			m_ChunkPos(chunkPos), m_BlockPos(blockPos) { }
	
	inline static BlockPos compute(const ChunkPos &chunkPos, const glm::ivec3 &blockPos) { // automatically adjusts chunkPos and blockPos if blockPos is out of range
		BlockPos out{chunkPos, blockPos};
		for(int dimension = 0; dimension < 3; dimension++) {
			while(out.m_BlockPos[dimension] < 0) {
				out.m_BlockPos[dimension] += 16;
				out.m_ChunkPos[dimension]--;
			}
			while(out.m_BlockPos[dimension] >= 16) {
				out.m_BlockPos[dimension] -= 16;
				out.m_ChunkPos[dimension]++;
			}
		}
		return out;
	}

public:
	inline ChunkPos chunkPos() const { return m_ChunkPos; }
	inline glm::ivec3 blockPos() const { return m_BlockPos; }

public:
	// inline BlockPos &operator+=(const glm::ivec3 &offset) {
	// 	m_BlockPos += offset;
	// 	for(int dimension = 0; dimension < 3; dimension++) {
	// 		if(m_BlockPos[dimension] >= 16.f) {
	// 			m_BlockPos[dimension] -= 16.f;
	// 			m_ChunkPos[dimension]++;
	// 		} else if(m_BlockPos[dimension] < 0.f) {
	// 			m_BlockPos[dimension] += 16.f;
	// 			m_ChunkPos[dimension]--;
	// 		}
	// 	}
	// 	return *this;
	// }
	// inline BlockPos &operator-=(const glm::vec3 &offset) {
	// 	return (*this) += -offset;
	// }
	// inline BlockPos operator+(const ChunkPos &offset) const {
	// 	return BlockPos{ m_ChunkPos + offset, m_BlockPos };
	// }
	// inline BlockPos operator-(const ChunkPos &offset) const {
	// 	return BlockPos{ m_ChunkPos - offset, m_BlockPos };
	// }
};










/*****************************************
 * --------   ENTITY POSITION   -------- *
 *****************************************/

class EntityPos {
private:
	ChunkPos m_ChunkPos; // Coordinates of containing Chunk
	glm::vec3 m_RelativePos; // Coordinates relative to chunk

public:
	inline EntityPos(const glm::vec3 &absolutePos):
			m_ChunkPos{
				(int64_t)std::floor(absolutePos.x / 16.f),
				(int64_t)std::floor(absolutePos.y / 16.f),
				(int64_t)std::floor(absolutePos.z / 16.f),
			},
			m_RelativePos(absolutePos - glm::floor(absolutePos / 16.f) * 16.f) {
		}
	inline EntityPos(const ChunkPos &chunkPos, const glm::vec3 &relativePos):
			m_ChunkPos(chunkPos), m_RelativePos(relativePos) { }
	
	inline static EntityPos compute(const ChunkPos &chunkPos, const glm::vec3 &relativePos) { // automatically adjusts chunkPos and blockPos if blockPos is out of range
		EntityPos out{chunkPos, relativePos};
		for(int dimension = 0; dimension < 3; dimension++) {
			while(out.m_RelativePos[dimension] < 0.f) {
				out.m_RelativePos[dimension] += 16.f;
				out.m_ChunkPos[dimension]--;
			}
			while(out.m_RelativePos[dimension] >= 16.f) {
				out.m_RelativePos[dimension] -= 16.f;
				out.m_ChunkPos[dimension]++;
			}
		}
		return out;
	}

public:
	inline ChunkPos chunkPos() const { return m_ChunkPos; }
	inline glm::vec3 relativePos() const { return m_RelativePos; }
	inline glm::vec3 computeAbsolute() const { return m_ChunkPos.toVec3() * 16.f + m_RelativePos; }

public:
	inline EntityPos &operator+=(const glm::vec3 &offset) {
		m_RelativePos += offset;
		for(int dimension = 0; dimension < 3; dimension++) {
			if(m_RelativePos[dimension] >= 16.f) {
				m_RelativePos[dimension] -= 16.f;
				m_ChunkPos[dimension]++;
			} else if(m_RelativePos[dimension] < 0.f) {
				m_RelativePos[dimension] += 16.f;
				m_ChunkPos[dimension]--;
			}
		}
		return *this;
	}
	inline EntityPos &operator-=(const glm::vec3 &offset) {
		return (*this) += -offset;
	}
	inline EntityPos operator+(const ChunkPos &offset) const {
		return EntityPos{ m_ChunkPos + offset, m_RelativePos };
	}
	inline EntityPos operator-(const ChunkPos &offset) const {
		return EntityPos{ m_ChunkPos - offset, m_RelativePos };
	}
};