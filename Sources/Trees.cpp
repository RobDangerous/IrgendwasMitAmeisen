#include "pch.h"
#include "Trees.h"

#include <Kore/Graphics4/Graphics.h>
#include <Kore/Graphics4/PipelineState.h>
#include <Kore/Graphics4/Texture.h>
#include <Kore/IO/FileReader.h>
#include <Kore/Math/Core.h>
#include <Kore/Math/Random.h>
#include <Kore/System.h>

#include "MeshObject.h"

using namespace Kore;
using namespace Kore::Graphics4;

const int maxTrees = 2;
MeshObject* trees[maxTrees];

namespace {
	Kore::Graphics4::VertexStructure structureTree;
	Kore::Graphics4::Shader* vertexShader;
	Kore::Graphics4::Shader* fragmentShader;
	Kore::Graphics4::PipelineState* pipeline;
	
	Kore::Graphics4::TextureUnit tex;
	Kore::Graphics4::ConstantLocation pLocation;
	Kore::Graphics4::ConstantLocation vLocation;
	Kore::Graphics4::ConstantLocation mLocation;
	Kore::Graphics4::ConstantLocation mInverseLocation;
	Kore::Graphics4::ConstantLocation diffuseLocation;
	Kore::Graphics4::ConstantLocation specularLocation;
	Kore::Graphics4::ConstantLocation specularPowerLocation;
	Kore::Graphics4::ConstantLocation lightPosLocation;
	Kore::Graphics4::ConstantLocation lightCount;
}

Trees::Trees() {

	loadShader();
	
	Kore::Quaternion treeRot = Kore::Quaternion(0, 0, 0, 1);
	treeRot.rotate(Kore::Quaternion(vec3(1, 0, 0), -Kore::pi / 2.0));
	
	trees[0] = new MeshObject("trees/tree02.ogex", "trees/", structureTree, 1.0);
	trees[0]->M = mat4::Translation(0, 1, 0) * treeRot.matrix().Transpose();
	
	trees[1] = new MeshObject("grass/grass.ogex", "grass/", structureTree, 1.0);
	trees[1]->M = mat4::Translation(0, 1, 0) * treeRot.matrix().Transpose();
}

void Trees::render(Kore::mat4 projectionMatrix, Kore::mat4 viewMatrix) {

	Graphics4::setPipeline(pipeline);
	
	Graphics4::setMatrix(vLocation, viewMatrix);
	Graphics4::setMatrix(pLocation, projectionMatrix);
	
	for (int i = 0; i < maxTrees; ++i) {
		trees[i]->setLights(lightCount, lightPosLocation);
		trees[i]->render(tex, mLocation, mInverseLocation, diffuseLocation, specularLocation, specularPowerLocation);
	}
}

void Trees::loadShader() {
	FileReader vs("shader_tree.vert");
	FileReader fs("shader_tree.frag");
	vertexShader = new Graphics4::Shader(vs.readAll(), vs.size(), Graphics4::VertexShader);
	fragmentShader = new Graphics4::Shader(fs.readAll(), fs.size(), Graphics4::FragmentShader);
	
	structureTree.add("pos", Graphics4::Float3VertexData);
	structureTree.add("tex", Graphics4::Float2VertexData);
	structureTree.add("nor", Graphics4::Float3VertexData);
	
	pipeline = new Graphics4::PipelineState();
	pipeline->inputLayout[0] = &structureTree;
	pipeline->inputLayout[1] = nullptr;
	pipeline->vertexShader = vertexShader;
	pipeline->fragmentShader = fragmentShader;
	pipeline->depthMode = Graphics4::ZCompareLess;
	pipeline->depthWrite = false;
	pipeline->blendSource = Graphics4::SourceAlpha;
	pipeline->blendDestination = Graphics4::InverseSourceAlpha;
	pipeline->alphaBlendSource = Graphics4::SourceAlpha;
	pipeline->alphaBlendDestination = Graphics4::InverseSourceAlpha;
	pipeline->compile();
	
	tex = pipeline->getTextureUnit("tex");
	Graphics4::setTextureAddressing(tex, Graphics4::U, Graphics4::Repeat);
	Graphics4::setTextureAddressing(tex, Graphics4::V, Graphics4::Repeat);
	
	pLocation = pipeline->getConstantLocation("P");
	vLocation = pipeline->getConstantLocation("V");
	mLocation = pipeline->getConstantLocation("M");
	mInverseLocation = pipeline->getConstantLocation("MInverse");
	diffuseLocation = pipeline->getConstantLocation("diffuseCol");
	specularLocation = pipeline->getConstantLocation("specularCol");
	specularPowerLocation = pipeline->getConstantLocation("specularPow");
	lightPosLocation = pipeline->getConstantLocation("lightPos");
	lightCount = pipeline->getConstantLocation("numLights");
}
