#pragma once
#include "pch.h"

#include <Kore/Math/Vector.h>
#include "GameObjects.h"

class MeshObject;

//return the island ID
int createIsland(Storage* storage, Kore::vec3 position, float radius, float ressources, MeshObject* navMesh);

//return bridge ID
int createBridge(Storage* storage, int islandIDfrom, int islandIDto);

void updateGameObjects(Storage* storage, float deltaTime);

float bridgeProgressPercentage(Bridge* bridge);

bool selectIsland(Storage* storage, Kore::vec3 rayStart, Kore::vec3 rayDir, IslandStruct* & selected);

int queenOnIsland(Storage* storage);

void queenPathFromIslandToIsland(Storage* storage, int islandIDfrom, int islandIDTo);