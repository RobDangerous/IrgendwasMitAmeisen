#include "pch.h"
#include "Island.h"

#include <Kore/Graphics4/Graphics.h>
#include <Kore/Graphics4/PipelineState.h>
#include <Kore/Graphics4/Texture.h>
#include <Kore/IO/FileReader.h>
#include <Kore/Math/Core.h>
#include <Kore/Math/Random.h>
#include <Kore/System.h>
#include <Kore/Log.h>

using namespace Kore;
using namespace Kore::Graphics4;

namespace {
	Kore::Graphics4::VertexStructure structure;
	Kore::Graphics4::Shader* vertexShader;
	Kore::Graphics4::Shader* fragmentShader;
	Kore::Graphics4::PipelineState* pipelineWithAlpha;
	Kore::Graphics4::PipelineState* pipelineWithoutAlpha;
	
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

Island::Island() {

	loadShaderWithAlpha();
	loadShaderWithoutAlpha();
	
	Kore::Quaternion rot = Kore::Quaternion(0, 0, 0, 1);
	rot.rotate(Kore::Quaternion(vec3(1, 0, 0), -Kore::pi / 2.0));
	
	islands[0] = new MeshObject("island/island.ogex", "island/", structure, 1.0);
	islands[0]->M = mat4::Translation(0, 0.6, 0) * rot.matrix().Transpose();
	islands[0]->xDim = 18.050; islands[0]->yDim = 17.821; islands[0]->zDim = 1.005;
	
	islands[1] = new MeshObject("island/island1.ogex", "island/", structure, 1.0);
	islands[1]->M = mat4::Translation(15, 0.6, 15) * rot.matrix().Transpose();
	islands[0]->xDim = 17.9354; islands[0]->yDim = 12.9319; islands[0]->zDim = 1.12583;
}

void Island::render(Kore::mat4 projectionMatrix, Kore::mat4 viewMatrix) {
	Graphics4::setPipeline(pipelineWithoutAlpha);
	
	Graphics4::setMatrix(vLocation, viewMatrix);
	Graphics4::setMatrix(pLocation, projectionMatrix);
	
	Graphics4::setTextureAddressing(tex, Graphics4::U, Graphics4::Repeat);
	Graphics4::setTextureAddressing(tex, Graphics4::V, Graphics4::Repeat);

	for (int i = 0; i < maxIslands; ++i) {
		islands[i]->setLights(lightCount, lightPosLocation);
		islands[i]->render(tex, mLocation, mInverseLocation, diffuseLocation, specularLocation, specularPowerLocation);
	}
}

void Island::loadShaderWithAlpha() {
	FileReader vs("shader_basic_lighting.vert");
	FileReader fs("shader_basic_lighting.frag");
	vertexShader = new Graphics4::Shader(vs.readAll(), vs.size(), Graphics4::VertexShader);
	fragmentShader = new Graphics4::Shader(fs.readAll(), fs.size(), Graphics4::FragmentShader);
	
	structure.add("pos", Graphics4::Float3VertexData);
	structure.add("tex", Graphics4::Float2VertexData);
	structure.add("nor", Graphics4::Float3VertexData);
	
	pipelineWithAlpha = new Graphics4::PipelineState();
	pipelineWithAlpha->inputLayout[0] = &structure;
	pipelineWithAlpha->inputLayout[1] = nullptr;
	pipelineWithAlpha->vertexShader = vertexShader;
	pipelineWithAlpha->fragmentShader = fragmentShader;
	pipelineWithAlpha->depthMode = Graphics4::ZCompareLess;
	pipelineWithAlpha->depthWrite = false;
	pipelineWithAlpha->blendSource = Graphics4::SourceAlpha;
	pipelineWithAlpha->blendDestination = Graphics4::InverseSourceAlpha;
	pipelineWithAlpha->alphaBlendSource = Graphics4::SourceAlpha;
	pipelineWithAlpha->alphaBlendDestination = Graphics4::InverseSourceAlpha;
	pipelineWithAlpha->compile();
	
	tex = pipelineWithAlpha->getTextureUnit("tex");
	Graphics4::setTextureAddressing(tex, Graphics4::U, Graphics4::Repeat);
	Graphics4::setTextureAddressing(tex, Graphics4::V, Graphics4::Repeat);
	
	pLocation = pipelineWithAlpha->getConstantLocation("P");
	vLocation = pipelineWithAlpha->getConstantLocation("V");
	mLocation = pipelineWithAlpha->getConstantLocation("M");
	mInverseLocation = pipelineWithAlpha->getConstantLocation("MInverse");
	diffuseLocation = pipelineWithAlpha->getConstantLocation("diffuseCol");
	specularLocation = pipelineWithAlpha->getConstantLocation("specularCol");
	specularPowerLocation = pipelineWithAlpha->getConstantLocation("specularPow");
	lightPosLocation = pipelineWithAlpha->getConstantLocation("lightPos");
	lightCount = pipelineWithAlpha->getConstantLocation("numLights");
}

void Island::loadShaderWithoutAlpha() {
	FileReader vs("shader_tree.vert");
	FileReader fs("shader_tree.frag");
	vertexShader = new Graphics4::Shader(vs.readAll(), vs.size(), Graphics4::VertexShader);
	fragmentShader = new Graphics4::Shader(fs.readAll(), fs.size(), Graphics4::FragmentShader);
	
	//structure.add("pos", Graphics4::Float3VertexData);
	//structure.add("tex", Graphics4::Float2VertexData);
	//structure.add("nor", Graphics4::Float3VertexData);
	
	pipelineWithoutAlpha = new Graphics4::PipelineState();
	pipelineWithoutAlpha->inputLayout[0] = &structure;
	pipelineWithoutAlpha->inputLayout[1] = nullptr;
	pipelineWithoutAlpha->vertexShader = vertexShader;
	pipelineWithoutAlpha->fragmentShader = fragmentShader;
	pipelineWithoutAlpha->depthMode = Graphics4::ZCompareLess;
	pipelineWithoutAlpha->depthWrite = true;
	pipelineWithoutAlpha->blendSource = Graphics4::SourceAlpha;
	pipelineWithoutAlpha->blendDestination = Graphics4::InverseSourceAlpha;
	pipelineWithoutAlpha->alphaBlendSource = Graphics4::SourceAlpha;
	pipelineWithoutAlpha->alphaBlendDestination = Graphics4::InverseSourceAlpha;
	pipelineWithoutAlpha->compile();
	
	/*tex = pipelineWithoutAlpha->getTextureUnit("tex");
	Graphics4::setTextureAddressing(tex, Graphics4::U, Graphics4::Repeat);
	Graphics4::setTextureAddressing(tex, Graphics4::V, Graphics4::Repeat);
	
	pLocation = pipelineWithoutAlpha->getConstantLocation("P");
	vLocation = pipelineWithoutAlpha->getConstantLocation("V");
	mLocation = pipelineWithoutAlpha->getConstantLocation("M");
	mInverseLocation = pipelineWithoutAlpha->getConstantLocation("MInverse");
	diffuseLocation = pipelineWithoutAlpha->getConstantLocation("diffuseCol");
	specularLocation = pipelineWithoutAlpha->getConstantLocation("specularCol");
	specularPowerLocation = pipelineWithoutAlpha->getConstantLocation("specularPow");
	lightPosLocation = pipelineWithoutAlpha->getConstantLocation("lightPos");
	lightCount = pipelineWithoutAlpha->getConstantLocation("numLights");*/
}
