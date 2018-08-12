#include "pch.h"

#include <Kore/IO/FileReader.h>
#include <Kore/Graphics1/Graphics.h>
#include <Kore/Graphics4/Graphics.h>
#include <Kore/Graphics4/PipelineState.h>
#include <Kore/Graphics4/Shader.h>
#include <Kore/System.h>
#include <Kore/Input/Keyboard.h>
#include <Kore/Input/Mouse.h>
#include <Kore/Log.h>

#include "Trees.h"
#include "Water.h"
#include "MeshObject.h"
#include "GameObjects.h"
#include "Systems.h"
#include "Ant.h"


using namespace Kore;

vec3 screenToWorldSpace(float posX, float posY);

namespace {
	const int width = 1024;
	const int height = 768;
	
	double startTime;
	double lastTime;
	
	const float CAMERA_NEAR_PLANE = 0.01f;
	const float CAMERA_FAR_PLANE = 1000;

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

	Graphics4::PipelineState* pipeline_living_room;
	Graphics4::TextureUnit tex_living_room;
	Graphics4::ConstantLocation pLocation_living_room;
	Graphics4::ConstantLocation vLocation_living_room;
	Graphics4::ConstantLocation mLocation_living_room;
	Graphics4::ConstantLocation mLocation_living_room_inverse;
	Graphics4::ConstantLocation diffuse_living_room;
	Graphics4::ConstantLocation specular_living_room;
	Graphics4::ConstantLocation specular_power_living_room;
	Graphics4::ConstantLocation lightPosLocation_living_room;
	Graphics4::ConstantLocation lightCount_living_room;
	
	bool renderTrees = true;


	void loadLivingRoomShader() {
		FileReader vs("shader_living_room.vert");
		FileReader fs("shader_living_room.frag");
		Graphics4::Shader* vertexShader_living_room = new Graphics4::Shader(vs.readAll(), vs.size(), Graphics4::VertexShader);
		Graphics4::Shader* fragmentShader_living_room = new Graphics4::Shader(fs.readAll(), fs.size(), Graphics4::FragmentShader);

		Graphics4::VertexStructure structure_living_room;
		structure_living_room.add("pos", Graphics4::Float3VertexData);
		structure_living_room.add("tex", Graphics4::Float2VertexData);
		structure_living_room.add("nor", Graphics4::Float3VertexData);

		pipeline_living_room = new Graphics4::PipelineState;
		pipeline_living_room->inputLayout[0] = &structure_living_room;
		pipeline_living_room->inputLayout[1] = nullptr;
		pipeline_living_room->vertexShader = vertexShader_living_room;
		pipeline_living_room->fragmentShader = fragmentShader_living_room;
		pipeline_living_room->depthMode = Graphics4::ZCompareLess;
		pipeline_living_room->depthWrite = true;
		pipeline_living_room->blendSource = Graphics4::SourceAlpha;
		pipeline_living_room->blendDestination = Graphics4::InverseSourceAlpha;
		pipeline_living_room->alphaBlendSource = Graphics4::SourceAlpha;
		pipeline_living_room->alphaBlendDestination = Graphics4::InverseSourceAlpha;
		pipeline_living_room->compile();

		tex_living_room = pipeline_living_room->getTextureUnit("tex");
		Graphics4::setTextureAddressing(tex_living_room, Graphics4::U, Graphics4::Repeat);
		Graphics4::setTextureAddressing(tex_living_room, Graphics4::V, Graphics4::Repeat);

		pLocation_living_room = pipeline_living_room->getConstantLocation("P");
		vLocation_living_room = pipeline_living_room->getConstantLocation("V");
		mLocation_living_room = pipeline_living_room->getConstantLocation("M");
		mLocation_living_room_inverse = pipeline_living_room->getConstantLocation("MInverse");
		diffuse_living_room = pipeline_living_room->getConstantLocation("diffuseCol");
		specular_living_room = pipeline_living_room->getConstantLocation("specularCol");
		specular_power_living_room = pipeline_living_room->getConstantLocation("specularPow");
		lightPosLocation_living_room = pipeline_living_room->getConstantLocation("lightPos");
		lightCount_living_room = pipeline_living_room->getConstantLocation("numLights");
	}

	Trees* trees;
	MeshObject* planet;
	MeshObject* bridge;
	Storage* storage;

	// Keyboard controls
	bool rotate = false;
	bool W, A, S, D = false;
	bool F, L, B, R = false;
	bool leftMouseDown = false;

	vec3 cameraUp;
	vec3 right;
	vec3 forward;
	float horizontalAngle = -1.24f * pi;
	float verticalAngle = -0.5f;
	
	vec3 cameraPos = vec3(0, 0, 0);
	

	Kore::mat4 getProjectionMatrix() {
		mat4 P = mat4::Perspective(45, (float)width / (float)height, CAMERA_NEAR_PLANE, CAMERA_FAR_PLANE);
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
		
		updateGameObjects(storage, deltaT);

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
		
		//mouse 
		if (leftMouseDown)
		{
			vec2i mousePos = System::mousePos();
			vec3 worldPosition = screenToWorldSpace(mousePos.x(), mousePos.y());
			Kore::log(Kore::LogLevel::Info, "Screen position x: %i y: %i to world position x: %f, y: %f, z %f", mousePos.x(), mousePos.y(), worldPosition.x(), worldPosition.y(), worldPosition.z());
			leftMouseDown = false;
		}
		
		Graphics4::begin();
		Graphics4::clear(Graphics4::ClearColorFlag | Graphics4::ClearDepthFlag, Kore::Graphics1::Color::Black, 1.0f, 0);
		
		mat4 P = getProjectionMatrix();
		mat4 V = getViewMatrix();

		renderWater(P * V, V, 0.0f);

		Graphics4::setPipeline(pipeline_living_room);
		//Ant::setLights(lightCount_living_room, lightPosLocation_living_room, livingRoom);
		Graphics4::setMatrix(vLocation_living_room, V);
		Graphics4::setMatrix(pLocation_living_room, P);
		Ant::render(tex_living_room, mLocation_living_room, mLocation_living_room_inverse, diffuse_living_room, specular_living_room, specular_power_living_room);

		if (renderTrees) {
			trees->render(P, V);
		}
		
		Graphics4::setPipeline(pipeline);
		Graphics4::setMatrix(vLocation, V);
		Graphics4::setMatrix(pLocation, P);

		
		//render islands
		for (int i = 0; i < storage->nextIsland; ++i) {
			vec3& islandPosition = storage->islands[i]->position;
			planet->setTransformation(mLocation, mat4::Translation(islandPosition.x(), islandPosition.y(), islandPosition.z()));
			planet->render(tex);
		}
		
		//render bridges
		for (int i = 0; i < storage->nextBridge; ++i) {
			Bridge* logicBridge = storage->bridges[i];
			vec3 islandFromPosition = storage->islands[logicBridge->islandIDfrom]->position;
			vec3 islandToPosition = storage->islands[logicBridge->islandIDto]->position;

			vec3 diff = islandToPosition - islandFromPosition;
			
			vec3 position = islandFromPosition;
			
			
			Kore::Quaternion rotation = Kore::Quaternion(0, 0, 0, 1);
			rotation.rotate(Kore::Quaternion(vec3(1, 0, 0), -Kore::pi / 2.0));
			
			float alpha = Kore::atan2(diff.x(), diff.z());
			
			rotation.rotate(Kore::Quaternion(vec3(0, 0, 1), alpha));

			

			mat4 scale = mat4::Scale(1.0f, -bridgeProgressPercentage(logicBridge) * diff.getLength(), 1.0f);

			bridge->setTransformation(mLocation, mat4::Translation(position.x(), position.y()+0.25f, position.z()) * rotation.matrix().Transpose() * scale);
			bridge->render(tex);
		}

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

void mouseDown(int windowId, int mouseButton, int x, int y)
{
	if (mouseButton == 0)
	{
		leftMouseDown = true;
	}
}

vec3 screenToWorldSpace(float posX, float posY)
{
	float xClip = (posX / width - 0.5f);
	float yClip = (posY / height - 0.5f);
	
	mat4 inverseProView = (getProjectionMatrix() * getViewMatrix()).Invert();

	vec4 positionClip(xClip, yClip, CAMERA_NEAR_PLANE, 1.0f);
	vec4 positionWorld = inverseProView * positionClip;
	positionWorld.x() /= positionWorld.w();
	positionWorld.y() /= positionWorld.w();
	positionWorld.z() /= positionWorld.w();

	return positionWorld.xyz();
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

void setUpGameLogic()
{
	storage = new Storage();
	int id0 = createIsland(storage, vec3(2, 1.0f, 2), 1, 100);
	int id1 = createIsland(storage, vec3(4, 1.0f, 4), 1, 100);
	storage->islands[id0]->antsOnIsland = 50;
	createBridge(storage, id0, id1);
}

int kore(int argc, char** argv) {
	Kore::System::init("Shader", width, height);
	Kore::System::setCallback(update);
	
	startTime = System::time();
	
	Keyboard::the()->KeyDown = keyDown;
	Keyboard::the()->KeyUp = keyUp;

	Mouse::the()->Press = mouseDown;

	trees = new Trees();
	
	loadShader();
	planet = new MeshObject("Sphere/sphere.ogex", "Sphere/", structure, 1.0);

	bridge = new MeshObject("AntBridge/AntBridge.ogex", "AntBridge/", structure, 1.0);

	cameraPos = vec3(-5, 5, 5);

	initWater();
	loadLivingRoomShader();
	Ant::init();
	
	setUpGameLogic();

	Kore::System::start();

	return 0;
}
