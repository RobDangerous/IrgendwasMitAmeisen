#include <Kore/pch.h>
#include "Systems.h"
#include <math.h>
#include <utility>
#include <Kore/Log.h>
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
	bridge->completedSinceSeconds = 0.0f;
	calcAntsNeededForBridge(storage, bridge);

	int id = storage->nextBridge++;
	bridge->id = id;
	storage->bridges[id] = bridge;
	return id;
}

void updateGameObjects(Storage* storage ,float deltaTime)
{
	//update islands with ant production and ressource gathering
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

		float antsMoved = deltaTime * bridge->length / antsValueSpeedPerSecond ;
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
	bridge->length = distance;
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
bool IntersectsWith(Kore::vec3 rayOrigin, Kore::vec3 rayDir, Kore::vec3 spherePos, float sphereRadius) {
	float t0, t1; // solutions for t if the ray intersects 
				  // geometric solution
	Kore::vec3 dirRayToSphere = spherePos - rayOrigin;
	float projectionDirCircleOnRay = dirRayToSphere * rayDir;
	// if (tca < 0) return false;
	float distRaySphereSquared = dirRayToSphere * dirRayToSphere - projectionDirCircleOnRay * projectionDirCircleOnRay; 
	if (distRaySphereSquared > sphereRadius * sphereRadius) 
		return false;
	else return true;
}

bool selectIsland(Storage* storage, Kore::vec3 rayStart, Kore::vec3 rayDir, IslandStruct* & selected)
{
	for (int i = 0; i < storage->nextIsland; ++i)
	{
		IslandStruct* island = storage->islands[i];
		if (IntersectsWith(rayStart,rayDir,island->position, island->radius))
		{
			selected = island;
			return true;
		}
	}
	return false;
}
