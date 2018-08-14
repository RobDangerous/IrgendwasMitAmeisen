
#include "Systems.h"
#include <math.h>
#include <utility>
#include <Kore/Log.h>
#include <limits>
#include "MeshObject.h"
#include "Ant.h"

		// Todo: tune me right
//bridge values
float antsNeededPerBridgeSizeValue = 10.0;
float bridgeValueSizePersecond = 0.3f;
float antsValueSpeedPerSecond = 1.5f;
//island values
float ressourceConsumptionPerAntPerSecond = 0.1f;
float antCreationPerSecond = 5.0f;
float antStarvationPerSecond = 0.5f;
//

void updateIsland(IslandStruct* island, float deltaTime);
void updateBridge(Bridge* bridge, Storage* storage, float deltaTime);
std::pair<IslandStruct*,IslandStruct*> getIslandWithMoreAnts(IslandStruct* islandA, IslandStruct* islandB);
bool isBridgeDone(Bridge* bridge);
void calcAntsNeededForBridge(Storage* storage, Bridge* bridge);
void moveQueen(AntQueen * queen, float deltaTime);
void createBridgeNavMeshBetweenIslands(BridgeNavMesh* bridgeNavMesh, IslandStruct* island0, IslandStruct* island1);
void createBridgeNavMeshPath(Bridge* bridge, Storage* storage);

std::vector<NavMeshNode*> meshNavPathFinding(NavMeshNode* startNode, NavMeshNode* endNode);

int createIsland(Storage* storage, Kore::vec3 position, float radius, float ressources, MeshObject* navMesh)
{
	IslandStruct* island = new IslandStruct();
	island->position = position;
	island->radius = radius;
	island->antsOnIsland = 0.0f;
	island->initialRessources = ressources;
	island->currentRessources = ressources;
	
	IslandNavMesh* islandNavMesh = new IslandNavMesh(navMesh->meshes[0], island, position + Kore::vec3(0,1.5f,0));
	island->navMesh = islandNavMesh;
	int id = storage->nextIsland++;
	island->id = id;
	storage->islands[id] = island;
	return id;
}

int createBridge(Storage* storage, int islandIDfrom, int islandIDto)
{
	Bridge* bridge = new Bridge();
	bridge->antsGathered = 0.0f;
	bridge->islandIDfrom = islandIDfrom;
	bridge->islandIDto = islandIDto;
	BridgeNavMesh* bridgeNavMesh = new BridgeNavMesh();
	createBridgeNavMeshBetweenIslands(bridgeNavMesh, storage->islands[islandIDfrom], storage->islands[islandIDto]);
	bridge->navMesh = bridgeNavMesh;
	bridgeNavMesh->bridge;
	calcAntsNeededForBridge(storage, bridge);
	int id = storage->nextBridge++;
	bridge->id = id;
	storage->bridges[id] = bridge;
	return id;
}

void updateGameObjects(Storage* storage ,float deltaTime)
{
	//update islands with ant production and ressource gathering

	moveQueen(storage->antQueen, deltaTime);

	for (int id = 0; id < storage->nextIsland; ++id)
	{
		updateIsland(storage->islands[id], deltaTime);
	}
		
	
	for (int id = 0; id < storage->nextBridge; ++id)
	{
		updateBridge(storage->bridges[id], storage, deltaTime);
	}
}

void updateIsland(IslandStruct* island, float deltaTime)
{
	if (island->antsOnIsland >= 1.0f)
	{
		if (island->currentRessources > 0.0f)
		{
			//create ants if ressources are available
			island->antsOnIsland += antCreationPerSecond * deltaTime;
			//Kore::log(Kore::LogLevel::Info, "Island %i has currently %f ants", island->id, island->antsOnIsland);
			//remove ressources per ant on island
			island->currentRessources -= floorf(island->antsOnIsland) * ressourceConsumptionPerAntPerSecond * deltaTime;
			//Kore::log(Kore::LogLevel::Info, "Island %i ressources reduced to %f", island->id, island->currentRessources);
		}
		else
		{
			//Kore::log(Kore::LogLevel::Info, "Island %i ants are starving", island->id);
			//ants starve
			island->antsOnIsland -= island->antsOnIsland * antStarvationPerSecond * deltaTime;
		}
	}
}

void updateBridge(Bridge* bridge, Storage* storage, float deltaTime)
{
	IslandStruct* fromIsland = storage->islands[bridge->islandIDfrom];
	if (isBridgeDone(bridge))
	{

		float antsMoved = deltaTime * bridge->completeBridgeLength / antsValueSpeedPerSecond ;
		//if bridges are already build, create ant equilibrium between connected islands
		IslandStruct* toIsland = storage->islands[bridge->islandIDto];
		
		std::pair<IslandStruct*, IslandStruct*> islandInhabitantsComparison = getIslandWithMoreAnts(fromIsland, toIsland);
		islandInhabitantsComparison.first->antsOnIsland -= antsMoved;
		islandInhabitantsComparison.second->antsOnIsland += antsMoved;
		//Kore::log(Kore::LogLevel::Info, "%f ants moved from island %i to island %i.", antsMoved, islandInhabitantsComparison.first->id, islandInhabitantsComparison.second->id);
	}
	else {
		//update bridge building with ants -> size
		float antsConsumedForBridge = antsNeededPerBridgeSizeValue * bridgeValueSizePersecond * deltaTime;
		if (fromIsland->antsOnIsland >= antsConsumedForBridge)
		{
			bridge->antsGathered += antsConsumedForBridge;
			Kore::log(Kore::LogLevel::Info, "Bridge %i is building and has %f ants on it.", bridge->id, bridge->antsGathered);
			fromIsland->antsOnIsland -= antsConsumedForBridge;
			//Kore::log(Kore::LogLevel::Info, "Bridge %i build removed %f ants from island %i", bridge->id, antsConsumedForBridge, bridge->islandIDfrom);
			if (isBridgeDone(bridge))
			{
				//create the navmesh nodes
				createBridgeNavMeshPath(bridge, storage);
			}
		}
		//else island does not have enough ants	
	}
}

std::pair<IslandStruct*, IslandStruct*> getIslandWithMoreAnts(IslandStruct* islandA, IslandStruct* islandB)
{
	if (islandA->antsOnIsland >= islandB->antsOnIsland)
		return std::pair<IslandStruct*,IslandStruct*>(islandA,islandB);
	else return std::pair<IslandStruct*, IslandStruct*>(islandB, islandA);
}

bool isBridgeDone(Bridge* bridge)
{
	return bridgeProgressPercentage(bridge) >= 1.0f;
}

void calcAntsNeededForBridge(Storage* storage, Bridge* bridge)
{
	float distance = bridgeCompleteLength(storage,bridge);
	bridge->completeBridgeLength = distance;
	float antsNeeded = ceil(distance * antsNeededPerBridgeSizeValue);
	bridge->antsNeeded = antsNeeded;
}

float bridgeProgressPercentage(Bridge* bridge)
{
	float percentage = bridge->antsGathered / (bridge->antsNeeded + 0.00001f);
	return Kore::min(percentage,1.0f);
}

// twice stolen from the internetz, guaranteed to work doubly good
// changed variable names for readability
// removed useless code
bool IntersectsWith(Kore::vec3 rayStart, Kore::vec3 rayDir, Kore::vec3 spherePos, float sphereRadius, float & out_distance) {
	Kore::vec3 dirRayStartToSphere = spherePos - rayStart;
	float distanceToSphere = dirRayStartToSphere.getLength();
	float projectionDirCircleOnRay = dirRayStartToSphere * rayDir;
	// if (tca < 0) return false;
	float collisionDepth = sphereRadius * sphereRadius -
		(distanceToSphere * distanceToSphere - projectionDirCircleOnRay * projectionDirCircleOnRay);
	if (collisionDepth < 0.0f)
	{
		return false;
	}
	else
	{
		out_distance = projectionDirCircleOnRay;
		return true;
	}
}

bool selectIsland(Storage* storage, Kore::vec3 rayStart, Kore::vec3 rayDir, IslandStruct* & selected)
{
	float closest = std::numeric_limits<float>::max();
	for (int i = 0; i < storage->nextIsland; ++i)
	{
		IslandStruct* island = storage->islands[i];
		float distance = std::numeric_limits<float>::max();
		if (IntersectsWith(rayStart,rayDir,island->position, island->radius, distance))

		{
			if (distance < closest)
			{
				selected = island;
				closest = distance;
			}
		}
	}
	if (closest < std::numeric_limits<float>::max())
		return true;
	else return false;
}

NavMeshNode* closestNavMeshNode(std::vector<NavMeshNode*> nodes, Kore::vec3 position)
{
	float closest = std::numeric_limits<float>::max();
	NavMeshNode* closestNode = nullptr;
	for (int i = 0; i < nodes.size(); ++i)
	{
		Kore::vec3 pos = nodes[i]->position;
		Kore::vec3 dist = position - pos;
		float squareDist = dist.squareLength();
		if (closest > squareDist)
		{
			closest = squareDist;
			closestNode = nodes[i];
		}
	}
	return closestNode;
}

void createBridgeNavMeshBetweenIslands(BridgeNavMesh* bridgeNavMesh, IslandStruct* island0, IslandStruct* island1)
{
	Kore::vec3 islandMidpoint = (island0->position + island1->position) * 0.5f;


	//find the closest point to the midpoint
	IslandNavMesh* navMesh0 = island0->navMesh;
	NavMeshNode* node0;

	IslandNavMesh* navMesh1 = island1->navMesh;
	NavMeshNode* node1;
	
	node0 = closestNavMeshNode(navMesh0->nodes, islandMidpoint);
	node1 = closestNavMeshNode(navMesh1->nodes, islandMidpoint);

	bridgeNavMesh->closestNodeIsland0 = node0;
	bridgeNavMesh->closestNodeIsland1 = node1;
	bridgeNavMesh->islandNavMesh0 = navMesh0;
	bridgeNavMesh->islandNavMesh1 = navMesh1;
}

void createBridgeNavMeshPath(Bridge* bridge, Storage* storage)
{
	std::vector<NavMeshNode*> bridgeNodes;
	int count = bridgeStepsCount(bridge);
	for (int i = 0; i < count; ++i)
	{
		Kore::vec3 position = bridgeStep(storage, bridge, i, bridgeCompleteStepsCount(bridge));
		NavMeshNode* bridgeNode = new NavMeshNode();
		bridgeNode->position = position;
		if (i > 0)
		{
			bridgeNode->neighbors.emplace_back(bridgeNodes.back());
			bridgeNodes.back()->neighbors.emplace_back(bridgeNode);
		}
		bridgeNodes.emplace_back(bridgeNode);
	}
	bridgeNodes[0]->neighbors.emplace_back(bridge->navMesh->closestNodeIsland0);
	bridge->navMesh->closestNodeIsland0->neighbors.emplace_back(bridgeNodes[0]);
	bridgeNodes.back()->neighbors.emplace_back(bridge->navMesh->closestNodeIsland1);
	bridge->navMesh->closestNodeIsland1->neighbors.emplace_back(bridgeNodes.back());
	bridge->navMesh->nodes = bridgeNodes;
}

int queenOnIsland(Storage* storage)
{
	Kore::vec3& queenPos = storage->antQueen->position;
	float queenRadius = storage->antQueen->radius;

	for (int i = 0; i < storage->nextIsland; ++i)
	{
		Kore::vec3& islandPos = storage->islands[i]->position;
		float islandRadius = storage->islands[i]->radius;

		float distSq = (queenPos - islandPos).squareLength();
		float radii = queenRadius + islandRadius;
		if (distSq <= radii * radii)
			return i;
	}
}

NavMeshNode* centerIslandNode(IslandStruct* island)
{
	auto nodes = island->navMesh->nodes;
	Kore::vec3& pos = island->position;

	NavMeshNode* centerNode = closestNavMeshNode(nodes, pos);
	return centerNode;
}

void queenPathFromIslandToIsland(Storage* storage, int islandIDfrom, int islandIDTo)
{
	//find if there is a bridge between the islands, else create one
	Bridge* bridge = nullptr;
	for (int i = 0; i < storage->nextBridge; ++i)
	{
		Bridge* b = storage->bridges[i];
		if((b->islandIDfrom == islandIDfrom && b->islandIDto == islandIDTo)
			|| (b->islandIDfrom == islandIDTo && b->islandIDto == islandIDfrom))
		{
			bridge = b;
		}
	}
	//if bridge, find closest node from queen to closest node on other island
	NavMeshNode* startNode = closestNavMeshNode(storage->islands[islandIDfrom]->navMesh->nodes,storage->antQueen->position);
	NavMeshNode* endNode = nullptr;
	if (bridge == nullptr)
	{
		//create a new bridge
		int bridgeID = createBridge(storage, islandIDfrom, islandIDTo);
		Bridge* bridge = storage->bridges[bridgeID];
		
		endNode = bridge->navMesh->closestNodeIsland0;
		//move queen to closestNavMeshNode
	}
	else
	{	
		//find path from node to node closest to island center
		endNode = closestNavMeshNode(storage->islands[islandIDTo]->navMesh->nodes, storage->islands[islandIDTo]->position);
	}
	storage->antQueen->path = meshNavPathFinding(startNode, endNode);
	storage->antQueen->pathIndex = 0;
}

void moveQueen(AntQueen * queen, float deltaTime)
{
	Kore::vec3 direction = queen->goalPoisition - queen->position;
	float distanceToGoal = direction.getLength();
	if (distanceToGoal < queen->goalReachedRadius)
	{
		//move quen to next path position
		if (!queen->path.empty() && queen->pathIndex < queen->path.size())
		{
			queen->goalPoisition = queen->path[queen->pathIndex]->position;
			queen->pathIndex += 1;
		}
		else {
			queen->position = queen->goalPoisition;
			queen->path.clear();
			queen->pathIndex = 0;
			return;
		}
	}
	
	float stepLength = queen->queenSpeedPerSecond * deltaTime;
	Kore::vec3 velocity = direction / (distanceToGoal +0.000001f);
	if (distanceToGoal < stepLength)
	{
		velocity *= distanceToGoal;
	}
	else {
		velocity *= stepLength;
	}
	queen->position += velocity;
	
}

typedef std::pair<NavMeshNode*, std::pair<Path*, float>> AStarNode;

//TODO: Path is broken
std::vector<NavMeshNode*> meshNavPathFinding(NavMeshNode* startNode, NavMeshNode* endNode)
{
	Path* startPath = new Path;
	startPath->node = startNode;
	startPath->previous = nullptr;
	std::vector<Path*> paths;
	paths.emplace_back(startPath);

	AStarNode startANode(startNode, std::pair<Path*, float>(startPath, 0));

	std::vector<NavMeshNode*> path;
	std::vector<AStarNode> visited;
	std::vector<AStarNode> open;
	
	open.emplace_back(startANode);

	auto getANodeIndex = [](NavMeshNode* node, std::vector<AStarNode>& list)
	{
		for (int i = 0; i < list.size(); ++i)
		{
			if (list[i].first == node)
			{
				return i;
			}
		}
		return -1;
	};

	auto placeNeighbors = [&](AStarNode& Anode)
	{
		NavMeshNode* node = Anode.first;
		for (int i = 0; i < node->neighbors.size(); ++i)
		{
			//check if neighbor is already in set
			
				int index = getANodeIndex(node->neighbors[i], open);
				if (index != -1)
				{
					auto neighbor = open[index];
					float distanceValue = (node->position - neighbor.first->position).squareLength() + Anode.second.second;
					if (neighbor.second.second > distanceValue)
					{
						neighbor.second.second = distanceValue;
						neighbor.second.first->node = node;
						neighbor.second.first->previous = Anode.second.first;
					}
				}
				else if (getANodeIndex(node->neighbors[i], visited) == -1)
				{
					Path* path = new Path();
					path->node = node->neighbors[i];
					path->previous = Anode.second.first;
					paths.emplace_back(path);
					AStarNode neighbor(node->neighbors[i], std::pair<Path*,float>(path, Anode.second.second));
					neighbor.second.second += (node->position - neighbor.first->position).squareLength();
					open.emplace_back(neighbor);
				}
		}
	};

	auto closestNeighbor = [&]()
	{
		float closest = std::numeric_limits<float>::max();
		
		int closestIndex = -1;
		for (int i = 0; i < open.size(); ++i)
		{
			if (open[i].second.second < closest)
			{
				closestIndex = open[i].second.second;
				closestIndex = i;
			}
		}
		AStarNode closestNode = open[closestIndex];

		open.erase(open.begin() + closestIndex);
		return closestNode;
	};

	auto buildPath = [&] (AStarNode& endNode)
	{
		Path* pathTree = endNode.second.first;
		while (pathTree->previous != nullptr)
		{
			path.insert(path.begin(), pathTree->node);
			pathTree = pathTree->previous;
		}
		for (int i = 0; i < paths.size(); ++i)
		{
			delete paths[i];
		}
	};

	while (!open.empty())
	{
		AStarNode Anode = closestNeighbor();


		if (Anode.first == endNode)
		{
			buildPath(Anode);
			return path;
		}

		visited.emplace_back(Anode);

		placeNeighbors(Anode);
	}
	return path;
}