#pragma once
#include "pch.h"
#include <Kore/Math/Vector.h>
#include <vector>

struct Mesh;
struct IslandStruct;
struct Bridge;

struct NavMeshNode;

struct NavMeshNode
{
	Kore::vec3 position;
	std::vector<NavMeshNode*> neighbors;
};

struct IslandNavMesh
{
	std::vector<NavMeshNode*> nodes;
	IslandStruct* island;
	IslandNavMesh(Mesh* navMesh, IslandStruct* island, Kore::vec3 offset);
};


struct BridgeNavMesh
{
	std::vector<NavMeshNode*> nodes;
	NavMeshNode* closestNodeIsland0;
	NavMeshNode* closestNodeIsland1;
	IslandNavMesh* islandNavMesh0;
	IslandNavMesh* islandNavMesh1;
	Bridge* bridge;
};

struct Path
{
	Path* previous;
	NavMeshNode* node;
};