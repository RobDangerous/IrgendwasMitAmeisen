#pragma once

#include <Kore/Math/Matrix.h>

class Trees {
public:
	Trees();
	
	void render(Kore::mat4 projectionMatrix, Kore::mat4 viewMatrix);
	
private:
	void loadShader();
};

