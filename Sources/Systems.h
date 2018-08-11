#pragma once
#include "pch.h"

#include <Kore/Math/Vector.h>
#include "gameObjects.h"

//return the island ID
int createIsland(Storage& storage, Kore::vec3 position, float radius, float ressources);

//return bridge ID
int createBridge(Storage& storage, int islandIDfrom, int islandIDto);

void updateGameObjects(Storage& storage, float deltaTime);
