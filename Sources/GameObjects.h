#pragma once

#include <Kore/Math/Vector.h>

template <typename T>
void initializeArrayOfPointers(T** arrayOfPointers, unsigned arraySize);

struct Island 
{
	int id;
	Kore::vec3 position;
	float radius;

	float initialRessources;
	float currentRessources;

	float antsOnIsland;
};

struct Bridge
{
	int islandIDfrom;
	int islandIDto;
	
	float length;
	float antsGathered;

	int completedSinceSeconds;
};

struct Storage
{
	int maxEntities = 255;

	Island** islands;
	int nextIsland;

	Bridge** bridges;
	int nextBridge;

	void setUp()
	{
		islands = new Island*[maxEntities];
		initializeArrayOfPointers(islands, maxEntities);
		nextIsland = 0;
		bridges = new Bridge*[maxEntities];
		initializeArrayOfPointers(bridges, maxEntities);
		nextBridge = 0;
	}

	Storage()
	{
		setUp();
	}
};

template <typename T>
void initializeArrayOfPointers(T** arrayOfPointers, unsigned arraySize)
{
	for (unsigned i = 0; i < arraySize; ++i)
	{
		arrayOfPointers[i] = nullptr;
	}
}
