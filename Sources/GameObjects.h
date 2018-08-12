#pragma once

#include <Kore/Math/Vector.h>

template <typename T>
void initializeArrayOfPointers(T** arrayOfPointers, unsigned arraySize);


struct IslandStruct
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
	int id;

	int islandIDfrom;
	int islandIDto;
	
	float antsNeeded;
	float antsGathered;
	float length;

	int completedSinceSeconds;
};

struct AntQueen
{
	Kore::vec3 position;
	float radius;

};

struct Storage
{
	int maxEntities = 255;

	IslandStruct** islands;
	int nextIsland;

	Bridge** bridges;
	int nextBridge;

	AntQueen* antQueen;

	void setUp()
	{
		islands = new IslandStruct*[maxEntities];
		initializeArrayOfPointers(islands, maxEntities);
		nextIsland = 0;
		bridges = new Bridge*[maxEntities];
		initializeArrayOfPointers(bridges, maxEntities);
		nextBridge = 0;
		antQueen = new AntQueen();
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
