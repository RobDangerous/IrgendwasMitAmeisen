#pragma once

#include <Kore/Math/Matrix.h>

class Island {
public:
	Island(const char* meshFile, const char* texturePath);
	
	void render(Kore::mat4 projectionMatrix, Kore::mat4 viewMatrix);
	
private:
	void loadShaderWithAlpha();
	void loadShaderWithoutAlpha();
};

