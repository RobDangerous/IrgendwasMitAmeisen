#include <Kore/pch.h>
#include "Systems.h"
#include <math.h>
#include <utility>
#include <Kore/Log.h>
#include <limits>
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

int createIsland(Storage* storage, Kore::vec3 position, float radius, float ressources)
{
	IslandStruct* island = new IslandStruct();
	island->position = position;
	island->radius = radius;
	island->antsOnIsland = 0.0f;
	island->initialRessources = ressources;
	island->currentRessources = ressources;
	
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
	calcAntsNeededForBridge(storage, bridge);
	BridgeNavMesh* bridgeNavMesh = new BridgeNavMesh();
//	createBridgeNavMeshBetweenIslands(bridgeNavMesh, storage->islands[islandIDfrom], storage->islands[islandIDto]);
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
			//Kore::log(Kore::LogLevel::Info, "Bridge %i is building and has %f ants on it.", bridge->id, bridge->antsGathered);
			fromIsland->antsOnIsland -= antsConsumedForBridge;
			//Kore::log(Kore::LogLevel::Info, "Bridge %i build removed %f ants from island %i", bridge->id, antsConsumedForBridge, bridge->islandIDfrom);
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
	Kore::vec3& islandPosFrom = storage->islands[bridge->islandIDfrom]->position;
	Kore::vec3& islandPosTo = storage->islands[bridge->islandIDto]->position;
	float distance = islandPosFrom.distance(islandPosTo);
	bridge->completeBridgeLength = distance;
	float antsNeeded = ceil(distance * antsNeededPerBridgeSizeValue);
	bridge->antsNeeded = antsNeeded;
}

float bridgeProgressPercentage(Bridge* bridge)
{
	float percentage = bridge->antsGathered / bridge->antsNeeded;
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
	for (int i = 0;nodes.size(); ++i)
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

	//create the bezier curve from bridge size
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

	if (bridge == nullptr)
	{
		//create a new bridge
	}
	else
	{	
		//find path from node to node closest to island center
		NavMeshNode* startNode = closestNavMeshNode(storage->islands[islandIDfrom]->navMesh->nodes,storage->antQueen->position);
		NavMeshNode* endNode = closestNavMeshNode(storage->islands[islandIDfrom]->navMesh->nodes, storage->islands[islandIDfrom]->position);
	}
}

void moveQueen(AntQueen * queen, float deltaTime)
{
	Kore::vec3 direction = queen->goalPoisition - queen->position;
	float distanceToGoal = direction.getLength();
	if (distanceToGoal < queen->goalReachedRadius)
	{
		queen->position = queen->goalPoisition;
	}
	else 
	{
		float stepLength = queen->queenSpeedPerSecond * deltaTime;
		Kore::vec3 velocity = direction / distanceToGoal;
		if (distanceToGoal < stepLength)
		{
			velocity *= distanceToGoal;
		}
		else {
			velocity *= stepLength;
		}
		queen->position += velocity;
	}
}

std::vector<NavMeshNode*> meshNavPathFinding(NavMeshNode* startNode, NavMeshNode* endNode)
{
	Path startPath;
	startPath.node = startNode;
	startPath.previous = nullptr;
	std::vector<Path> paths;
	paths.emplace_back(startPath);
	Path* pathTree = &paths[0];
	std::pair<NavMeshNode*, float> startANode(startNode, 0);

	std::vector<NavMeshNode*> path;
	std::vector<std::pair<NavMeshNode*, float>> visited;
	std::vector<std::pair<NavMeshNode*,float>> open;
	
	open.emplace_back(startANode);

	auto getANodeIndex = [](NavMeshNode* node, std::vector<std::pair<NavMeshNode*, float>>& list)
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

	auto placeNeighbors = [&](std::pair<NavMeshNode*,float>& Anode)
	{
		NavMeshNode* node = Anode.first;
		for (int i = 0; i < node->neighbors.size(); ++i)
		{
			//check if neighbor is already in set
			if (getANodeIndex(node->neighbors[i], visited) != -1)
			{
				int index = getANodeIndex(node->neighbors[i], open);
				if (index != -1)
				{
					auto neighbor = open[index];
					float distanceValue = (node->position - neighbor.first->position).squareLength() + Anode.second;
					if (neighbor.second > distanceValue)
					{
						neighbor.second = distanceValue;
					}
				}
				else {
					std::pair<NavMeshNode*, float> neighbor(node->neighbors[i], Anode.second);
					neighbor.second += (node->position - neighbor.first->position).squareLength();
					open.emplace_back(neighbor);
				}
			}
		}
	};

	auto closestNeighbor = [&]()
	{
		float closest = std::numeric_limits<float>::max();
		
		int closestIndex = -1;
		for (int i = 0; i < open.size(); ++i)
		{
			if (open[i].second < closest)
			{
				closestIndex = open[i].second;
				closestIndex = i;
			}
		}
		std::pair<NavMeshNode*, float> closestNode = open[closestIndex];

		open.erase(open.begin() + closestIndex);
		return closestNode;
	};

	auto buildPath = [&] ()
	{
		while (pathTree->previous != nullptr)
		{
			path.insert(path.begin(), pathTree->node);
			pathTree = pathTree->previous;
		}
	};

	while (!open.empty())
	{
		std::pair<NavMeshNode*, float> Anode = closestNeighbor();

		Path lastPath;
		lastPath.node = Anode.first;
		lastPath.previous = pathTree;
		paths.emplace_back(lastPath);
		pathTree = &paths[paths.size() - 1];

		if (Anode.first == endNode)
		{
			buildPath;
			return path;
		}

		visited.emplace_back(Anode);

		placeNeighbors(Anode);
	}
	buildPath;
	return path;
}