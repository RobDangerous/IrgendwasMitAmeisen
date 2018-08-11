#pragma once

#include <Kore/Math/Matrix.h>

class Baum {
public:
	Baum(const char* meshFile, const char* texturePath, float scale = 1.0);
	
	void render(Kore::mat4 projectionMatrix, Kore::mat4 viewMatrix);
	
private:
	void loadShader();
};

