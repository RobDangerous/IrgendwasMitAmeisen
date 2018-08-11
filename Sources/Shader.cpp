#include "pch.h"

#include <Kore/IO/FileReader.h>
#include <Kore/Graphics1/Graphics.h>
#include <Kore/Graphics4/Graphics.h>
#include <Kore/Graphics4/PipelineState.h>
#include <Kore/Graphics4/Shader.h>
#include <Kore/System.h>
#include <Kore/Input/Keyboard.h>
#include <Kore/Log.h>

#include "Baum.h"
#include "water.h"
#include "MeshObject.h"


using namespace Kore;

namespace {
	const int width = 1024;
	const int height = 768;
	
	double startTime;
	double lastTime;
	
	const float CAMERA_ROTATION_SPEED = 0.05f;
	const float CAMERA_MOVE_SPEED = 4.f;
	
	// Simple shader
	Graphics4::VertexStructure structure;
	Graphics4::Shader* vertexShader;
	Graphics4::Shader* fragmentShader;
	Graphics4::PipelineState* pipeline;
	
	Graphics4::TextureUnit tex;
	Graphics4::ConstantLocation pLocation;
	Graphics4::ConstantLocation vLocation;
	Graphics4::ConstantLocation mLocation;

	
	bool renderTrees = true;

	Baum* tree;
	MeshObject* planet;
	Baum* tree2;

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
		Graphics4::clear(Graphics4::ClearColorFlag | Graphics4::ClearDepthFlag, Kore::Graphics1::Color::Black, 1.0f, 0);
		
		mat4 P = getProjectionMatrix();
		mat4 V = getViewMatrix();

		renderWater(P * V, V, 0.0f);
		

		if (renderTrees) {
			tree->render(P, V);
			//tree2->render(P, V);
		}
		
		Graphics4::setPipeline(pipeline);
		Graphics4::setMatrix(vLocation, V);
		Graphics4::setMatrix(pLocation, P);
		
		Graphics4::setMatrix(mLocation, planet->M);
		planet->render(tex);
		
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

void loadShader() {
	FileReader vs("shader.vert");
	FileReader fs("shader.frag");
	vertexShader = new Graphics4::Shader(vs.readAll(), vs.size(), Graphics4::VertexShader);
	fragmentShader = new Graphics4::Shader(fs.readAll(), fs.size(), Graphics4::FragmentShader);
	
	// This defines the structure of your Vertex Buffer
	structure.add("pos", Graphics4::Float3VertexData);
	structure.add("tex", Graphics4::Float2VertexData);
	structure.add("nor", Graphics4::Float3VertexData);
	
	pipeline = new Graphics4::PipelineState();
	pipeline->inputLayout[0] = &structure;
	pipeline->inputLayout[1] = nullptr;
	pipeline->vertexShader = vertexShader;
	pipeline->fragmentShader = fragmentShader;
	pipeline->depthMode = Graphics4::ZCompareLess;
	pipeline->depthWrite = true;
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
}

int kore(int argc, char** argv) {
	Kore::System::init("Shader", width, height);
	Kore::System::setCallback(update);
	
	startTime = System::time();
	
	Keyboard::the()->KeyDown = keyDown;
	Keyboard::the()->KeyUp = keyUp;


	tree = new Baum("Tree02/tree02.ogex", "Tree02/");
	
	loadShader();
	planet = new MeshObject("Sphere/sphere.ogex", "Sphere/", structure, 1.0);
	//tree2 = new Baum("tree_stump/pine_tree.ogex", "tree_stump/");

	/*AntBridge = new MeshObject("AntBridge/AntBridge.ogex", "AntBridge/", vertex_structure, 1.0f);
	rotateBlenderMesh(AntBridge);
	translateMeshObject(AntBridge, -2, 2, -2);*/

	cameraPos = vec3(-5, 5, 5);

	initWater();
	
	Kore::System::start();

	return 0;
}
