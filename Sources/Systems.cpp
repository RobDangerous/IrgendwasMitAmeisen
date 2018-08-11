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

void updateIsland(Island* island, float deltaTime);
void updateBridge(Bridge* bridge, Storage* storage, float deltaTime);
std::pair<Island*,Island*> getIslandWithMoreAnts(Island* islandA, Island* islandB);
bool isBridgeDone(float bridgeLength, float antsGathered);
float calcAntsNeededForBridge(float bridgeLength);
float currentBridgeLength(Bridge* bridge);

int createIsland(Storage* storage, Kore::vec3 position, float radius, float ressources)
{
	Island* island = new Island();
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
	Kore::vec3& islandPosFrom = storage->islands[islandIDfrom]->position;
	Kore::vec3& islandPosTo = storage->islands[islandIDto]->position;

	Bridge* bridge = new Bridge();
	bridge->length = islandPosFrom.distance(islandPosTo);
	bridge->antsGathered = 0.0f;
	bridge->islandIDfrom = islandIDfrom;
	bridge->islandIDto = islandIDto;
	bridge->completedSinceSeconds = 0.0f;

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

void updateIsland(Island* island, float deltaTime)
{
	if (island->antsOnIsland >= 1.0f)
	{
		if (island->currentRessources > 0.0f)
		{
			//create ants if ressources are available
			island->antsOnIsland += antCreationPerSecond * deltaTime;
			Kore::log(Kore::LogLevel::Info, "Island %i has currently %f ants", island->id, island->antsOnIsland);
			//remove ressources per ant on island
			island->currentRessources -= floorf(island->antsOnIsland) * ressourceConsumptionPerAntPerSecond * deltaTime;
			Kore::log(Kore::LogLevel::Info, "Island %i ressources reduced to %f", island->id, island->currentRessources);
		}
		else
		{
			Kore::log(Kore::LogLevel::Info, "Island %i ants are starving", island->id);
			//ants starve
			island->antsOnIsland -= island->antsOnIsland * antStarvationPerSecond * deltaTime;
		}
	}
}

void updateBridge(Bridge* bridge, Storage* storage, float deltaTime)
{
	Island* fromIsland = storage->islands[bridge->islandIDfrom];
	if (isBridgeDone(bridge->length, bridge->antsGathered))
	{

		float antsMoved = deltaTime * bridge->length / antsValueSpeedPerSecond ;
		//if bridges are already build, create ant equilibrium between connected islands
		Island* toIsland = storage->islands[bridge->islandIDto];
		
		std::pair<Island*, Island*> islandInhabitantsComparison = getIslandWithMoreAnts(fromIsland, toIsland);
		islandInhabitantsComparison.first->antsOnIsland -= antsMoved;
		islandInhabitantsComparison.second->antsOnIsland += antsMoved;
		Kore::log(Kore::LogLevel::Info, "%f ants moved from island %i to island %i.", antsMoved, islandInhabitantsComparison.first->id, islandInhabitantsComparison.second->id);
	}
	else {
		//update bridge building with ants -> size
		float antsConsumedForBridge = antsNeededPerBridgeSizeValue * bridgeValueSizePersecond * deltaTime;
		if (fromIsland->antsOnIsland >= antsConsumedForBridge)
		{
			bridge->antsGathered += antsConsumedForBridge;
			Kore::log(Kore::LogLevel::Info, "Bridge %i is building and has %f ants on it.", bridge->id, bridge->antsGathered);
			fromIsland->antsOnIsland -= antsConsumedForBridge;
			Kore::log(Kore::LogLevel::Info, "Bridge %i build removed %f ants from island %i", bridge->id, antsConsumedForBridge, bridge->islandIDfrom);
		}
		//else island does not have enough ants	
	}
}

std::pair<Island*, Island*> getIslandWithMoreAnts(Island* islandA, Island* islandB)
{
	if (islandA->antsOnIsland >= islandB->antsOnIsland)
		return std::pair<Island*,Island*>(islandA,islandB);
	else return std::pair<Island*, Island*>(islandB, islandA);
}

bool isBridgeDone(float bridgeLength, float antsGathered)
{
	return calcAntsNeededForBridge(bridgeLength) <= antsGathered;
}

float calcAntsNeededForBridge(float bridgeLength)
{
	return ceil(bridgeLength * antsNeededPerBridgeSizeValue);
}

float currentBridgeLength(Bridge* bridge)
{
	float percentage = calcAntsNeededForBridge(bridge->length) / bridge->antsGathered;
	return percentage * bridge->length;
}