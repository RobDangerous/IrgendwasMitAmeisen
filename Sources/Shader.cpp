#include "pch.h"

#include <Kore/IO/FileReader.h>
#include <Kore/Graphics1/Graphics.h>
#include <Kore/Graphics4/Graphics.h>
#include <Kore/Graphics4/PipelineState.h>
#include <Kore/Graphics4/Shader.h>
#include <Kore/System.h>
#include <Kore/Input/Keyboard.h>
#include <Kore/Log.h>

#include "MeshObject.h"
#include "water.h"


using namespace Kore;

namespace {
	const int width = 1024;
	const int height = 768;
	
	double startTime;
	double lastTime;
	
	const float CAMERA_ROTATION_SPEED = 0.05f;
	const float CAMERA_MOVE_SPEED = 4.f;
	
	Graphics4::Shader* vertexShader;
	Graphics4::Shader* fragmentShader;
	Graphics4::PipelineState* pipeline;
	Graphics4::VertexBuffer* vertices;
	Graphics4::IndexBuffer* indices;
	
	// Tree shader
	bool renderTrees = true;
	Graphics4::VertexStructure vertex_structure;
	Graphics4::Shader* vertexShader_tree;
	Graphics4::Shader* fragmentShader_tree;
	Graphics4::PipelineState* pipeline_mesh;
	
	Graphics4::TextureUnit tex_tree;
	Graphics4::ConstantLocation pLocation_tree;
	Graphics4::ConstantLocation vLocation_tree;
	Graphics4::ConstantLocation mLocation_tree;
	Graphics4::ConstantLocation mLocation_tree_inverse;
	Graphics4::ConstantLocation diffuse_tree;
	Graphics4::ConstantLocation specular_tree;
	Graphics4::ConstantLocation specular_power_tree;
	Graphics4::ConstantLocation lightPosLocation_tree;
	Graphics4::ConstantLocation lightCount_tree;
	
	MeshObject* tree;
	MeshObject* sphere;
	MeshObject* AntBridge;
	// Keyboard controls
	bool rotate = false;
	bool W, A, S, D = false;
	bool F, L, B, R = false;
	
	vec3 cameraUp;
	vec3 right;
	vec3 forward;
	float horizontalAngle = -1.24f * pi;
	float verticalAngle = -0.5f;
	
	vec3 cameraPos = vec3(0, 0, 0);
	
	Kore::mat4 getProjectionMatrix() {
		mat4 P = mat4::Perspective(45, (float)width / (float)height, 0.01f, 1000);
		P.Set(0, 0, -P.get(0, 0));
		return P;
	}
	
	Kore::mat4 getViewMatrix() {
		// Calculate camera direction
		vec3 cameraDir = vec3(Kore::cos(verticalAngle) * Kore::sin(horizontalAngle), Kore::sin(verticalAngle), Kore::cos(verticalAngle) * Kore::cos(horizontalAngle));
		
		// Re-calculate the orthonormal up vector
		cameraUp = right.cross(forward);  // cross product
		cameraUp.normalize();
		
		mat4 V = mat4::lookAlong(cameraDir, cameraPos, cameraUp);
		return V;
	}

	void update() {
		double t = System::time() - startTime;
		double deltaT = t - lastTime;
		lastTime = t;
		
		cameraUp = vec3(0, 1, 0);
		right = vec3(Kore::sin(horizontalAngle - pi / 2.0), 0, Kore::cos(horizontalAngle - pi / 2.0));
		forward = cameraUp.cross(right);  // cross product
		
		// Move position of camera based on WASD keys
		if (S || B) {
			cameraPos -= forward * (float) deltaT * CAMERA_MOVE_SPEED;
		}
		if (W || F) {
			cameraPos += forward * (float) deltaT * CAMERA_MOVE_SPEED;
		}
		if (A || L) {
			cameraPos -= right * (float)deltaT * CAMERA_MOVE_SPEED;
		}
		if (D || R) {
			cameraPos += right * (float)deltaT * CAMERA_MOVE_SPEED;
		}
		
		
		Graphics4::begin();
		Graphics4::clear(Graphics4::ClearColorFlag | Graphics4::ClearDepthFlag, Kore::Graphics1::Color::Green, 1.0f, 0);
		
		if (renderTrees) {
			mat4 P = getProjectionMatrix();
			mat4 V = getViewMatrix();
		
			Graphics4::setPipeline(pipeline_mesh);
			
			Graphics4::setMatrix(vLocation_tree, V);
			Graphics4::setMatrix(pLocation_tree, P);
			
			tree->setLights(lightCount_tree, lightPosLocation_tree);
			tree->render(tex_tree, mLocation_tree, mLocation_tree_inverse, diffuse_tree, specular_tree, specular_power_tree);

			sphere->setLights(lightCount_tree, lightPosLocation_tree);
			sphere->render(tex_tree, mLocation_tree, mLocation_tree_inverse, diffuse_tree, specular_tree, specular_power_tree);

			AntBridge->setLights(lightCount_tree, lightPosLocation_tree);
			AntBridge->render(tex_tree, mLocation_tree, mLocation_tree_inverse, diffuse_tree, specular_tree, specular_power_tree);
		}

		renderWater(getProjectionMatrix() * getViewMatrix(), 0.0f);

		Graphics4::end();
		Graphics4::swapBuffers();
	}
}

void keyDown(KeyCode code) {
	switch (code) {
		case Kore::KeyW:
			W = true;
			break;
		case Kore::KeyA:
			A = true;
			break;
		case Kore::KeyS:
			S = true;
			break;
		case Kore::KeyD:
			D = true;
			break;
		case Kore::KeyLeft:
			L = true;
			break;
		case Kore::KeyRight:
			R = true;
			break;
		case Kore::KeyUp:
			F = true;
			break;
		case Kore::KeyDown:
			B = true;
			break;
		case Kore::KeySpace:
			break;
		case Kore::KeyR:
			break;
		case KeyL:
			Kore::log(Kore::LogLevel::Info, "Position: (%f, %f, %f)", cameraPos.x(), cameraPos.y(), cameraPos.z());
			Kore::log(Kore::LogLevel::Info, "Rotation: (%f, %f)", verticalAngle, horizontalAngle);
			break;
		case Kore::KeyEscape:
		case KeyQ:
			System::stop();
			break;
		default:
			break;
	}
}

void keyUp(KeyCode code) {
	switch (code) {
		case Kore::KeyW:
			W = false;
			break;
		case Kore::KeyA:
			A = false;
			break;
		case Kore::KeyS:
			S = false;
			break;
		case Kore::KeyD:
			D = false;
			break;
		case Kore::KeyLeft:
			L = false;
			break;
		case Kore::KeyRight:
			R = false;
			break;
		case Kore::KeyUp:
			F = false;
			break;
		case Kore::KeyDown:
			B = false;
			break;
		default:
			break;
	}
}

void loadTreeShader() {
	FileReader vs("shader_tree.vert");
	FileReader fs("shader_tree.frag");
	vertexShader_tree = new Graphics4::Shader(vs.readAll(), vs.size(), Graphics4::VertexShader);
	fragmentShader_tree = new Graphics4::Shader(fs.readAll(), fs.size(), Graphics4::FragmentShader);
	
	vertex_structure.add("pos", Graphics4::Float3VertexData);
	vertex_structure.add("tex", Graphics4::Float2VertexData);
	vertex_structure.add("nor", Graphics4::Float3VertexData);
	
	pipeline_mesh = new Graphics4::PipelineState();
	pipeline_mesh->inputLayout[0] = &vertex_structure;
	pipeline_mesh->inputLayout[1] = nullptr;
	pipeline_mesh->vertexShader = vertexShader_tree;
	pipeline_mesh->fragmentShader = fragmentShader_tree;
	pipeline_mesh->depthMode = Graphics4::ZCompareLess;
	pipeline_mesh->depthWrite = true;
	pipeline_mesh->blendSource = Graphics4::SourceAlpha;
	pipeline_mesh->blendDestination = Graphics4::InverseSourceAlpha;
	pipeline_mesh->alphaBlendSource = Graphics4::SourceAlpha;
	pipeline_mesh->alphaBlendDestination = Graphics4::InverseSourceAlpha;
	pipeline_mesh->compile();
	
	tex_tree = pipeline_mesh->getTextureUnit("tex");
	Graphics4::setTextureAddressing(tex_tree, Graphics4::U, Graphics4::Repeat);
	Graphics4::setTextureAddressing(tex_tree, Graphics4::V, Graphics4::Repeat);
	
	pLocation_tree = pipeline_mesh->getConstantLocation("P");
	vLocation_tree = pipeline_mesh->getConstantLocation("V");
	mLocation_tree = pipeline_mesh->getConstantLocation("M");
	mLocation_tree_inverse = pipeline_mesh->getConstantLocation("MInverse");
	diffuse_tree = pipeline_mesh->getConstantLocation("diffuseCol");
	specular_tree = pipeline_mesh->getConstantLocation("specularCol");
	specular_power_tree = pipeline_mesh->getConstantLocation("specularPow");
	lightPosLocation_tree = pipeline_mesh->getConstantLocation("lightPos");
	lightCount_tree = pipeline_mesh->getConstantLocation("numLights");
}

void rotateBlenderMesh(MeshObject* blenderMesh)
{
	Kore::Quaternion blenderMeshRotation = Kore::Quaternion(0, 0, 0, 1);
	blenderMeshRotation.rotate(Kore::Quaternion(vec3(1, 0, 0), -Kore::pi / 2.0));
	blenderMeshRotation.rotate(Kore::Quaternion(vec3(0, 0, 1), Kore::pi / 2.0));
	blenderMesh->M = mat4::Translation(0, 0, 0) * blenderMeshRotation.matrix().Transpose();
}

void translateMeshObject(MeshObject* blenderMesh, float x, float y, float z)
{
	blenderMesh->M = blenderMesh->M * mat4::Translation(x,y,z).Transpose();
}

void scaleMeshObject(MeshObject* mesh, float scale)
{
	mesh->M = mesh->M * mat4::Scale(scale).Transpose();
}

int kore(int argc, char** argv) {
	Kore::System::init("Shader", width, height);
	Kore::System::setCallback(update);
	
	startTime = System::time();
	
	Keyboard::the()->KeyDown = keyDown;
	Keyboard::the()->KeyUp = keyUp;

	loadTreeShader();
	tree = new MeshObject("Tree02/tree02.ogex", "Tree02/", vertex_structure, 1.0f);
	rotateBlenderMesh(tree);
	
	sphere = new MeshObject("Sphere/sphere.ogex", "Sphere/", vertex_structure, 1.0f);
	scaleMeshObject(sphere, 0.1f);
	rotateBlenderMesh(sphere);
	translateMeshObject(sphere, 3, 2, 3);

	/*AntBridge = new MeshObject("AntBridge/AntBridge.ogex", "AntBridge/", vertex_structure, 1.0f);
	rotateBlenderMesh(AntBridge);
	translateMeshObject(AntBridge, -2, 2, -2);*/

	cameraPos = vec3(-5, 5, 5);

	initWater();
	
	Kore::System::start();

	return 0;
}
