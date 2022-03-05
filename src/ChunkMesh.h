#pragma once

#include "Coordinates.h"
#include "Mesh.h"
#include "GPUMesh.h"

#include "ChunkMeshGenerator.h"

class ChunkMesh {
public:
	ChunkPos sourceChunk;
	Mesh mesh;
	GPUMesh gpuMesh;

public:
	inline ChunkMesh(const ChunkPos &sourceChunk):
		sourceChunk(sourceChunk),
		mesh{},
		gpuMesh{} { }

	ChunkMesh(const ChunkMesh&) = delete;
	ChunkMesh &operator=(const ChunkMesh&) = delete;

	ChunkMesh(ChunkMesh&& source):
			sourceChunk(source.sourceChunk) {
		mesh = std::move(source.mesh);
		gpuMesh = std::move(source.gpuMesh);
	}
	ChunkMesh &operator=(ChunkMesh&& source) {
		if (this != &source) {
			sourceChunk = source.sourceChunk;
			mesh = std::move(source.mesh);
			gpuMesh = std::move(source.gpuMesh);
        }
        return *this;
	}

public:
	inline void generate(const World &world) {
		generateChunkMesh(world, sourceChunk, mesh);
		gpuMesh.upload(mesh);
	}
};