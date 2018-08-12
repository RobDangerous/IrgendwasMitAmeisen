#pragma once

#include <Kore/Math/Matrix.h>

class Island {
public:
	Island();
	
	void render(Kore::mat4 projectionMatrix, Kore::mat4 viewMatrix);
	
private:
	void loadShaderWithAlpha();
	void loadShaderWithoutAlpha();
};

