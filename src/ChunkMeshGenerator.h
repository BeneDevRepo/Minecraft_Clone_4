#pragma once

#include "Coordinates.h"
#include "Mesh.h"

class World;

void generateChunkMesh(const World &world, const ChunkPos &chunkPos, Mesh &mesh);