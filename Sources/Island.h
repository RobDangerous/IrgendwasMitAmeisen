#pragma once

#include <Kore/Math/Matrix.h>

#include "MeshObject.h"

class Island {
public:
	Island();
	
	static const int maxIslands = 2;
	MeshObject* islands[maxIslands];
	
	void render(Kore::mat4 projectionMatrix, Kore::mat4 viewMatrix);
	
private:
	void loadShaderWithAlpha();
	void loadShaderWithoutAlpha();
};

