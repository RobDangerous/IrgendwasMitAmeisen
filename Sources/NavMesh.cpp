#include "NavMesh.h"
#include "MeshObject.h"
#include "GameObjects.h"

IslandNavMesh::IslandNavMesh(Mesh* navMesh, IslandStruct* island, Kore::vec3 offset)
{
	this->island = island;
	//get faces/triangles 
	//get vertices, and create triangle centers
	std::vector<NavMeshNode*> nodes;
	std::vector<Kore::vec3i> triangles;
	for (int i = 0; i < navMesh->numFaces; ++i)
	{
		Kore::vec3 center(0,0,0);
		for (int t = 0; t < 3; ++t)
		{
			int index = navMesh->indices[t + i];
			center.x() += navMesh->vertices[index];
			center.y() += navMesh->vertices[index + 1];
			center.z() += navMesh->vertices[index + 2];
			Kore::vec3i triangle(index, index + 1, index + 2);
			triangles.emplace_back(triangle);
		}
		center /= 3;
		NavMeshNode* navNode = new NavMeshNode();
		navNode->position = center + offset;
		nodes.emplace_back(navNode);
	}
	//get neighboring vertices of faces to create connections between face centers
	auto connects = [&](int index, Kore::vec3i triangle) 
	{
		for (int i = 0; i < 3; ++i)
		{
			if (triangle[i] == index)
				return true;
		}
	};
	std::vector<Kore::vec2i> edges;
	for (int i = 0; i < triangles.size(); ++i)
	{
		Kore::vec3i triangle = triangles[i];
		for (int v = 0; v < 3; ++v)
		{
			int vertexIndex = triangle[v];
			for (int j = i + 1; j < triangles.size(); ++j)
			{
				if (connects(vertexIndex, triangles[j]))
				{
					//create edge
					nodes[i]->neighbors.emplace_back(nodes[j]);
					nodes[j]->neighbors.emplace_back(nodes[i]);
				}
			}
		}
	}
	this->nodes = nodes;
}