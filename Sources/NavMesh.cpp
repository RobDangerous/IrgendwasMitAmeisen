#include "NavMesh.h"
#include "MeshObject.h"
#include "GameObjects.h"

IslandNavMesh::IslandNavMesh(Mesh* navMesh, IslandStruct* island, Kore::vec3 offset)
{

	Kore::vec3 nodepositions[] = { Kore::vec3(0,0,0), Kore::vec3(2,0,0), Kore::vec3(4,0,0),
		Kore::vec3(6,0,0), Kore::vec3(0,0,2), Kore::vec3(0,0,4), Kore::vec3(0,0,6),
		Kore::vec3(2,0,2), Kore::vec3(4,0,2), Kore::vec3(6,0,2),
		Kore::vec3(2,0,4), Kore::vec3(2,0,6), Kore::vec3(4,0,4),
		Kore::vec3(4,0,6), Kore::vec3(6,0,4), Kore::vec3(6,0,6) };

	std::vector<NavMeshNode*> nodes;

	for (int i = 0; i < 16; ++i)
	{
		NavMeshNode* node = new NavMeshNode();
		node->position = nodepositions[i] + offset;
		nodes.emplace_back(node);
	}

	for (int i = 0; i < nodes.size(); ++i)
	{
		NavMeshNode* node = nodes[i];
		for (int j = i +1; j < nodes.size(); ++j)
		{
			NavMeshNode* otherNode = nodes[j];
			if ((node->position - otherNode->position).squareLength() < 4.1f)
			{
				node->neighbors.emplace_back(otherNode);
				otherNode->neighbors.emplace_back(node);
			}
		}
	}
	this->island = island;
	this->nodes = nodes;
	/*Kore::Quaternion rot = Kore::Quaternion(0, 0, 0, 1);
	rot.rotate(Kore::Quaternion(Kore::vec3(1, 0, 0), -Kore::pi / 2.0));
	Kore::mat4 rotation = rot.matrix();
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
		}
		center /= 3;
		Kore::vec4 rotatedCenter(center,1);
		rotatedCenter = rotation * rotatedCenter;
		NavMeshNode* navNode = new NavMeshNode();
		navNode->position = rotatedCenter.xyz() + offset;
		nodes.emplace_back(navNode);
		
		Kore::vec3i triangle(navMesh->indices[i], navMesh->indices[1 + i], navMesh->indices[2 + i]);
		triangles.emplace_back(triangle);
	}
	
	//get neighboring vertices of faces to create connections between face centers
	auto connects = [&](int index, Kore::vec3i triangle) 
	{
		for (int i = 0; i < 3; ++i)
		{
			if (triangle[i] == index)
				return true;
		}
		return false;
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
	*/
}