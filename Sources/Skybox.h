#pragma once

#include "MeshObject.h"

class Skybox {
	public:
	
	Skybox();
	
	MeshObject* getSkybox();

	private:
	
	void loadShader();
};
