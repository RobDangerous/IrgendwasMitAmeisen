#include "pch.h"
#include "Baum.h"

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

namespace {
	MeshObject* meshObject;
	
	Kore::Graphics4::VertexStructure structure_tree;
	Kore::Graphics4::Shader* vertexShader_tree;
	Kore::Graphics4::Shader* fragmentShader_tree;
	Kore::Graphics4::PipelineState* pipeline_tree;
	
	Kore::Graphics4::TextureUnit tex_tree;
	Kore::Graphics4::ConstantLocation pLocation_tree;
	Kore::Graphics4::ConstantLocation vLocation_tree;
	Kore::Graphics4::ConstantLocation mLocation_tree;
	Kore::Graphics4::ConstantLocation mLocation_tree_inverse;
	Kore::Graphics4::ConstantLocation diffuse_tree;
	Kore::Graphics4::ConstantLocation specular_tree;
	Kore::Graphics4::ConstantLocation specular_power_tree;
	Kore::Graphics4::ConstantLocation lightPosLocation_tree;
	Kore::Graphics4::ConstantLocation lightCount_tree;
}

Baum::Baum(const char* meshFile, const char* texturePath) {

	loadShader();
	
	meshObject = new MeshObject(meshFile, texturePath, structure_tree, 1.0);
	Kore::Quaternion treeRot = Kore::Quaternion(0, 0, 0, 1);
	treeRot.rotate(Kore::Quaternion(vec3(1, 0, 0), -Kore::pi / 2.0));
	treeRot.rotate(Kore::Quaternion(vec3(0, 0, 1), Kore::pi / 2.0));
	meshObject->M = mat4::Translation(0, 0, 0) * treeRot.matrix().Transpose();
}

void Baum::render(Kore::mat4 projectionMatrix, Kore::mat4 viewMatrix) {

	Graphics4::setPipeline(pipeline_tree);
	
	Graphics4::setMatrix(vLocation_tree, viewMatrix);
	Graphics4::setMatrix(pLocation_tree, projectionMatrix);
	
	meshObject->setLights(lightCount_tree, lightPosLocation_tree);
	meshObject->render(tex_tree, mLocation_tree, mLocation_tree_inverse, diffuse_tree, specular_tree, specular_power_tree);
}


void Baum::loadShader() {
	FileReader vs("shader_tree.vert");
	FileReader fs("shader_tree.frag");
	vertexShader_tree = new Graphics4::Shader(vs.readAll(), vs.size(), Graphics4::VertexShader);
	fragmentShader_tree = new Graphics4::Shader(fs.readAll(), fs.size(), Graphics4::FragmentShader);
	
	structure_tree.add("pos", Graphics4::Float3VertexData);
	structure_tree.add("tex", Graphics4::Float2VertexData);
	structure_tree.add("nor", Graphics4::Float3VertexData);
	
	pipeline_tree = new Graphics4::PipelineState();
	pipeline_tree->inputLayout[0] = &structure_tree;
	pipeline_tree->inputLayout[1] = nullptr;
	pipeline_tree->vertexShader = vertexShader_tree;
	pipeline_tree->fragmentShader = fragmentShader_tree;
	pipeline_tree->depthMode = Graphics4::ZCompareLess;
	pipeline_tree->depthWrite = false;
	pipeline_tree->blendSource = Graphics4::SourceAlpha;
	pipeline_tree->blendDestination = Graphics4::InverseSourceAlpha;
	pipeline_tree->alphaBlendSource = Graphics4::SourceAlpha;
	pipeline_tree->alphaBlendDestination = Graphics4::InverseSourceAlpha;
	pipeline_tree->compile();
	
	tex_tree = pipeline_tree->getTextureUnit("tex");
	Graphics4::setTextureAddressing(tex_tree, Graphics4::U, Graphics4::Repeat);
	Graphics4::setTextureAddressing(tex_tree, Graphics4::V, Graphics4::Repeat);
	
	pLocation_tree = pipeline_tree->getConstantLocation("P");
	vLocation_tree = pipeline_tree->getConstantLocation("V");
	mLocation_tree = pipeline_tree->getConstantLocation("M");
	mLocation_tree_inverse = pipeline_tree->getConstantLocation("MInverse");
	diffuse_tree = pipeline_tree->getConstantLocation("diffuseCol");
	specular_tree = pipeline_tree->getConstantLocation("specularCol");
	specular_power_tree = pipeline_tree->getConstantLocation("specularPow");
	lightPosLocation_tree = pipeline_tree->getConstantLocation("lightPos");
	lightCount_tree = pipeline_tree->getConstantLocation("numLights");
}
