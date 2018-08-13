#pragma once

#include "MeshObject.h"

class Skybox {
	public:
	
	Skybox();
	
	void getSkybox(const Kore::Graphics4::VertexStructure& structure);
	void render(Kore::Graphics4::TextureUnit tex);

	private:
	
};
