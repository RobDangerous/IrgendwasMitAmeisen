#include "pch.h"

#include "Skybox.h"

#include <Kore/Math/Vector.h>

namespace {
	MeshObject* skybox;
	Kore::Graphics4::Texture* image;
	
	Kore::Graphics4::VertexBuffer* vertexBuffer;
	Kore::Graphics4::IndexBuffer* indexBuffer;
}

Skybox::Skybox() {
	image = new Kore::Graphics4::Texture("island/skybox.jpg", true);
}


void Skybox::getSkybox(const Kore::Graphics4::VertexStructure& structure) {
	
	// Create Vertex Buffer
	vertexBuffer = new Kore::Graphics4::VertexBuffer(24, structure);
	float* v = vertexBuffer->lock();
	int index = 0;
	
	v[index++] = -scale; v[index++] = -scale; v[index++] = -scale; v[index++] = 0.0; v[index++] = 2.0 / 3.0; v[index++] = 0; v[index++] = 0; v[index++] = 0;
	v[index++] = -scale; v[index++] = scale; v[index++] = -scale; v[index++] = 0.0; v[index++] = 1.0 / 3.0; v[index++] = 0; v[index++] = 0; v[index++] = 0;
	v[index++] = -scale; v[index++] = scale; v[index++] = scale; v[index++] = 1.0 / 4.0; v[index++] = 1.0 / 3.0; v[index++] = 0; v[index++] = 0; v[index++] = 0;
	v[index++] = -scale; v[index++] = -scale; v[index++] = scale; v[index++] = 1.0 / 4.0; v[index++] = 2.0 / 3.0; v[index++] = 0; v[index++] = 0; v[index++] = 0;
	
	v[index++] = -scale; v[index++] = -scale; v[index++] = scale; v[index++] = 1.0 / 4.0; v[index++] = 2.0 / 3.0; v[index++] = 0; v[index++] = 0; v[index++] = 0;
	v[index++] = -scale; v[index++] = scale; v[index++] = scale; v[index++] = 1.0 / 4.0; v[index++] = 1.0 / 3.0; v[index++] = 0; v[index++] = 0; v[index++] = 0;
	v[index++] = scale; v[index++] = scale; v[index++] = scale; v[index++] = 2.0 / 4.0; v[index++] = 1.0 / 3.0; v[index++] = 0; v[index++] = 0; v[index++] = 0;
	v[index++] = scale; v[index++] = -scale; v[index++] = scale; v[index++] = 2.0 / 4.0; v[index++] = 2.0 / 3.0; v[index++] = 0; v[index++] = 0; v[index++] = 0;
	
	v[index++] = -scale; v[index++] = -scale; v[index++] = -scale; v[index++] = 1.0 / 4.0; v[index++] = 1; v[index++] = 0; v[index++] = 0; v[index++] = 0;
	v[index++] = -scale; v[index++] = -scale; v[index++] = scale; v[index++] = 1.0 / 4.0; v[index++] = 2.0 / 3.0; v[index++] = 0; v[index++] = 0; v[index++] = 0;
	v[index++] = scale; v[index++] = -scale; v[index++] = scale; v[index++] = 2.0 / 4.0; v[index++] = 2.0 / 3.0; v[index++] = 0; v[index++] = 0; v[index++] = 0;
	v[index++] = scale; v[index++] = -scale; v[index++] = -scale; v[index++] = 2.0 / 4.0; v[index++] = 1; v[index++] = 0; v[index++] = 0; v[index++] = 0;
	
	v[index++] = -scale; v[index++] = scale; v[index++] = scale; v[index++] = 1.0 / 4.0; v[index++] = 1.0 / 3.0; v[index++] = 0; v[index++] = 0; v[index++] = 0;
	v[index++] = -scale; v[index++] = scale; v[index++] = -scale; v[index++] = 1.0 / 4.0; v[index++] = 0; v[index++] = 0; v[index++] = 0; v[index++] = 0;
	v[index++] = scale; v[index++] = scale; v[index++] = -scale; v[index++] = 2.0 / 4.0; v[index++] = 0; v[index++] = 0; v[index++] = 0; v[index++] = 0;
	v[index++] = scale; v[index++] = scale; v[index++] = scale; v[index++] = 2.0 / 4.0; v[index++] = 1.0 / 3.0; v[index++] = 0; v[index++] = 0; v[index++] = 0;
	
	v[index++] = scale; v[index++] = -scale; v[index++] = scale; v[index++] = 2.0 / 4.0; v[index++] = 2.0 / 3.0; v[index++] = 0; v[index++] = 0; v[index++] = 0;
	v[index++] = scale; v[index++] = scale; v[index++] = scale; v[index++] = 2.0 / 4.0; v[index++] = 1.0 / 3.0; v[index++] = 0; v[index++] = 0; v[index++] = 0;
	v[index++] = scale; v[index++] = scale; v[index++] = -scale; v[index++] = 3.0 / 4.0; v[index++] = 1.0 / 3.0; v[index++] = 0; v[index++] = 0; v[index++] = 0;
	v[index++] = scale; v[index++] = -scale; v[index++] = -scale; v[index++] = 3.0 / 4.0; v[index++] = 2.0 / 3.0; v[index++] = 0; v[index++] = 0; v[index++] = 0;
	
	v[index++] = scale; v[index++] = -scale; v[index++] = -scale; v[index++] = 3.0 / 4.0; v[index++] = 2.0 / 3.0; v[index++] = 0; v[index++] = 0; v[index++] = 0;
	v[index++] = scale; v[index++] = scale; v[index++] = -scale; v[index++] = 3.0 / 4.0; v[index++] = 1.0 / 3.0; v[index++] = 0; v[index++] = 0; v[index++] = 0;
	v[index++] = -scale; v[index++] = scale; v[index++] = -scale; v[index++] = 1.0; v[index++] = 1.0 / 3.0; v[index++] = 0; v[index++] = 0; v[index++] = 0;
	v[index++] = -scale; v[index++] = -scale; v[index++] = -scale; v[index++] = 1.0; v[index++] = 2.0 / 3.0; v[index++] = 0; v[index++] = 0; v[index++] = 0;
	
	vertexBuffer->unlock();
	
	// Create Index Buffer
	indexBuffer = new Kore::Graphics4::IndexBuffer(36);
	int* indices = indexBuffer->lock();
	
	index = 0;
	int vert = 0;
	for (int i = 0; i < 6; i++) {
		indices[index++] = vert++; indices[index++] = vert++; indices[index++] = vert++;
		indices[index++] = vert++; indices[index++] = vert - 4; indices[index++] = vert - 2;
	}
	
	indexBuffer->unlock();
}

void Skybox::render(Kore::Graphics4::TextureUnit tex) {
	Kore::Graphics4::setTexture(tex, image);
	Kore::Graphics4::setVertexBuffer(*vertexBuffer);
	Kore::Graphics4::setIndexBuffer(*indexBuffer);
	Kore::Graphics4::drawIndexedVertices();
}

float Skybox::getSkyboxScale() {
	return scale;
}



