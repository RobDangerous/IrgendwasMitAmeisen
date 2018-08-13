#include "pch.h"

#include <Kore/IO/FileReader.h>
#include <Kore/Graphics1/Graphics.h>
#include <Kore/Graphics2/Graphics.h>
#include <Kore/Graphics4/Graphics.h>
#include <Kore/Graphics4/PipelineState.h>
#include <Kore/Graphics4/Shader.h>
#include <Kore/System.h>
#include <Kore/Input/Keyboard.h>
#include <Kore/Input/Mouse.h>
#include <Kore/Log.h>

#include "Island.h"
#include "Water.h"
#include "MeshObject.h"
#include "GameObjects.h"
#include "Systems.h"
#include "Ant.h"
#include "Skybox.h"

using namespace Kore;


namespace {
	const int width = 1024;
	const int height = 768;
	
	double startTime;
	double lastTime;
	
	const float CAMERA_NEAR_PLANE = 0.01f;
	const float CAMERA_FAR_PLANE = 1000;
	
	const float CAMERA_ROTATION_SPEED = 0.05f;
	const float CAMERA_MOVE_SPEED = 4.f;
	
	const float queenHeightOffset = 1.0f;

	vec3 screenToWorldSpace(float posX, float posY);
	// Simple shader
	Graphics4::VertexStructure structure;
	Graphics4::Shader* vertexShader;
	Graphics4::Shader* fragmentShader;
	Graphics4::PipelineState* pipeline;
	
	Graphics4::TextureUnit tex;
	Graphics4::ConstantLocation pLocation;
	Graphics4::ConstantLocation vLocation;
	Graphics4::ConstantLocation mLocation;

	Graphics4::PipelineState* pipeline_basic_lighting;
	Graphics4::TextureUnit tex_basic_lighting;
	Graphics4::ConstantLocation pLocation_basic_lighting;
	Graphics4::ConstantLocation vLocation_basic_lighting;
	Graphics4::ConstantLocation mLocation_basic_lighting;
	Graphics4::ConstantLocation mLocation_basic_lighting_inverse;
	Graphics4::ConstantLocation diffuse_basic_lighting;
	Graphics4::ConstantLocation specular_basic_lighting;
	Graphics4::ConstantLocation specular_power_basic_lighting;
	Graphics4::ConstantLocation lightPosLocation_basic_lighting;
	Graphics4::ConstantLocation lightCount_basic_lighting;

	Graphics4::Texture* queenTex;
	
	Graphics4::Texture* antTexture;
	Graphics4::Texture* treeTexture;

	void loadShaderBasicLighting() {
		FileReader vs("shader_basic_lighting.vert");
		FileReader fs("shader_basic_lighting.frag");
		Graphics4::Shader* vertexShader_basic_lighting = new Graphics4::Shader(vs.readAll(), vs.size(), Graphics4::VertexShader);
		Graphics4::Shader* fragmentShader_basic_lighting = new Graphics4::Shader(fs.readAll(), fs.size(), Graphics4::FragmentShader);

		Graphics4::VertexStructure structure_basic_lighting;
		structure_basic_lighting.add("pos", Graphics4::Float3VertexData);
		structure_basic_lighting.add("tex", Graphics4::Float2VertexData);
		structure_basic_lighting.add("nor", Graphics4::Float3VertexData);

		pipeline_basic_lighting = new Graphics4::PipelineState;
		pipeline_basic_lighting->inputLayout[0] = &structure_basic_lighting;
		pipeline_basic_lighting->inputLayout[1] = nullptr;
		pipeline_basic_lighting->vertexShader = vertexShader_basic_lighting;
		pipeline_basic_lighting->fragmentShader = fragmentShader_basic_lighting;
		pipeline_basic_lighting->depthMode = Graphics4::ZCompareLess;
		pipeline_basic_lighting->depthWrite = true;
		pipeline_basic_lighting->blendSource = Graphics4::SourceAlpha;
		pipeline_basic_lighting->blendDestination = Graphics4::InverseSourceAlpha;
		pipeline_basic_lighting->alphaBlendSource = Graphics4::SourceAlpha;
		pipeline_basic_lighting->alphaBlendDestination = Graphics4::InverseSourceAlpha;
		pipeline_basic_lighting->compile();

		tex_basic_lighting = pipeline_basic_lighting->getTextureUnit("tex");

		pLocation_basic_lighting = pipeline_basic_lighting->getConstantLocation("P");
		vLocation_basic_lighting = pipeline_basic_lighting->getConstantLocation("V");
		mLocation_basic_lighting = pipeline_basic_lighting->getConstantLocation("M");
		mLocation_basic_lighting_inverse = pipeline_basic_lighting->getConstantLocation("MInverse");
		diffuse_basic_lighting = pipeline_basic_lighting->getConstantLocation("diffuseCol");
		specular_basic_lighting = pipeline_basic_lighting->getConstantLocation("specularCol");
		specular_power_basic_lighting = pipeline_basic_lighting->getConstantLocation("specularPow");
		lightPosLocation_basic_lighting = pipeline_basic_lighting->getConstantLocation("lightPos");
		lightCount_basic_lighting = pipeline_basic_lighting->getConstantLocation("numLights");
	}

	Skybox* skybox;
	
	Island* island;
	
	MeshObject* planet;
	MeshObject* bridge;
	Storage* storage;
	
	Kore::Graphics2::Graphics2* g2;
	Kravur* font14;
	Kravur* font24;
	Kravur* font34;
	Kravur* font44;

	// Keyboard controls
	bool rotate = false;
	bool W, A, S, D = false;
	bool F, L, B, R = false;
	bool leftMouseDown = false;

	/*vec3 cameraUp;
	vec3 right;
	vec3 forward;
	float horizontalAngle = -1.24f * pi;
	float verticalAngle = -0.5f;*/

	vec4 camUp(0.0f, 1.0f, 0.0f, 0.0f);
	vec4 camForward(0.0f, 0.0f, 1.0f, 0.0f);
	vec4 camRight(1.0f, 0.0f, 0.0f, 0.0f);
	
	vec3 cameraPos = vec3(0, 0, 0);

	Kore::mat4 getProjectionMatrix() {
		mat4 P = mat4::Perspective(45, (float)width / (float)height, CAMERA_NEAR_PLANE, CAMERA_FAR_PLANE);
		P.Set(0, 0, -P.get(0, 0));
		return P;
	}
	
	Kore::mat4 getViewMatrix() {
		mat4 V = mat4::lookAlong(camForward.xyz(), cameraPos, vec3(0.0f, 1.0f, 0.0f));
		return V;
	}

	void update() {
		double t = System::time() - startTime;
		double deltaT = t - lastTime;
		lastTime = t;
		
		updateGameObjects(storage, deltaT);
		Ant::moveEverybody(storage, deltaT);

		/*cameraUp = vec3(0, 1, 0);
		right = vec3(Kore::sin(horizontalAngle - pi / 2.0), 0, Kore::cos(horizontalAngle - pi / 2.0));
		
		forward = cameraUp.cross(right);  // cross product*/
		
		// Move position of camera based on WASD keys
		if (S || B) {
			cameraPos -= camForward * (float) deltaT * CAMERA_MOVE_SPEED;
		}
		if (W || F) {
			cameraPos += camForward * (float) deltaT * CAMERA_MOVE_SPEED;
		}
		if (A || L) {
			cameraPos += camRight * (float)deltaT * CAMERA_MOVE_SPEED;
		}
		if (D || R) {
			cameraPos -= camRight * (float)deltaT * CAMERA_MOVE_SPEED;
		}
		
		//mouse
		if (leftMouseDown)
		{
			/*vec2i mousePos = System::mousePos();
			vec3 worldPosition = cameraPos;//screenToWorldSpace(mousePos.x(), mousePos.y());
			Kore::log(Kore::LogLevel::Info, "Screen position x: %i y: %i to world position x: %f, y: %f, z %f", mousePos.x(), mousePos.y(), worldPosition.x(), worldPosition.y(), worldPosition.z());
			*/
			vec3 rayDir = camForward;
			rayDir.normalize();

			IslandStruct* selected = nullptr;
			if (selectIsland(storage, cameraPos, rayDir, selected))
			{
				Kore::log(Kore::LogLevel::Info, "Selected Island %i",selected->id);
				vec3 queenGoalPosition = selected->position + vec3(0, queenHeightOffset, 0);
				storage->antQueen->goalPoisition = queenGoalPosition;
			}
			leftMouseDown = false;

			
		}
		
		Graphics4::begin();
		Graphics4::clear(Graphics4::ClearColorFlag | Graphics4::ClearDepthFlag, Kore::Graphics1::Color::Green, 1.0f, 0);
		
		mat4 P = getProjectionMatrix();
		mat4 V = getViewMatrix();
		
		Graphics4::setPipeline(pipeline);
		Graphics4::setMatrix(vLocation, V);
		Graphics4::setMatrix(pLocation, P);
		
		Graphics4::setMatrix(mLocation, mat4::Identity());
		skybox->render(tex);
		
		renderWater(P * V, V, cameraPos, 0.0f);

		Graphics4::setPipeline(pipeline_basic_lighting);
		Ant::setLights(lightCount_basic_lighting, lightPosLocation_basic_lighting);
		Graphics4::setMatrix(vLocation_basic_lighting, V);
		Graphics4::setMatrix(pLocation_basic_lighting, P);
		Ant::render(tex_basic_lighting, mLocation_basic_lighting, mLocation_basic_lighting_inverse, diffuse_basic_lighting, specular_basic_lighting, specular_power_basic_lighting);

		island->render(P, V);
		//island->riseSeaLevel(deltaT);
		
		/*Graphics4::setPipeline(pipeline);
		Graphics4::setMatrix(vLocation, V);
		Graphics4::setMatrix(pLocation, P);
		//render islands
		for (int i = 0; i < storage->nextIsland; ++i) {
			vec3& islandPosition = storage->islands[i]->position;
			planet->setTransformation(mLocation, mat4::Translation(islandPosition.x(), islandPosition.y(), islandPosition.z()) * mat4::Scale(storage->islands[i]->radius));
			planet->render(tex);
		}*/

		//render queen
		AntQueen* antqueen = storage->antQueen;
		planet->setTransformation(mLocation, mat4::Translation(antqueen->position.x(), antqueen->position.y(), antqueen->position.z()) * mat4::Scale(antqueen->radius).Transpose());
		Graphics4::setTexture(tex, queenTex);

		Graphics4::setVertexBuffer(*planet->vertexBuffers[0]);
		Graphics4::setIndexBuffer(*planet->indexBuffers[0]);
		Graphics4::drawIndexedVertices();
		planet->render(tex);

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
		
		
		g2->begin(false, width, height, false);
		
		g2->setImageScaleQuality(Graphics2::High);
		// Show current ant count
		g2->drawImage(antTexture, 10, 10);
		g2->setFont(font44);
		char c1[42];
		sprintf(c1, "%i", currentAnts);
		g2->drawString(c1, 120, 10);
		
		// Show resources
		g2->drawImage(treeTexture, 40, 60);
		g2->setFont(font44);
		char c2[42];
		sprintf(c2, "todo");
		g2->drawString(c2, 120, 60);
		
		g2->end();

		Graphics4::end();
		Graphics4::swapBuffers();
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
				Kore::log(Kore::LogLevel::Info, "Looking at: (%f, %f %f %f)", camForward.x(), camForward.y(), camForward.z(), camForward.w());
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


	vec3 screenToWorldSpace(float posX, float posY)
	{
		float xClip = (2*posX / width) - 1.0f;
		float yClip = -((2*posY / height) - 1.0f);
	
		mat4 inverseProView = getViewMatrix().Invert() * getProjectionMatrix().Invert();

		vec4 positionClip(xClip, yClip, 0, 1.0f);
		vec4 positionWorld = inverseProView * positionClip;
		positionWorld /= positionWorld.w();

		return positionWorld.xyz();
	}

	double lastMouseTime = 0;
	void mouseMove(int windowId, int x, int y, int movementX, int movementY) {
		double t = System::time() - startTime;
		double deltaT = t - lastMouseTime;
		lastMouseTime = t;
		
		Quaternion q1(vec3(0.0f, 1.0f, 0.0f), 0.01f * movementX);
		Quaternion q2(camRight, 0.01f * -movementY);

		camUp = q2.matrix() * camUp;
		camRight = q1.matrix() * camRight;

		q1.rotate(q2);
		mat4 mat = q1.matrix();
		camForward = mat * camForward;
	}

	
	void mousePress(int windowId, int button, int x, int y) {
		rotate = true;

		if (button == 0)
		{
			leftMouseDown = true;
		}
	}
	
	void mouseRelease(int windowId, int button, int x, int y) {
		rotate = false;
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
		
		Kore::vec3 center;
		float radius;
		island->islands[0]->getBoundingBox(&center, &radius);
		int id0 = createIsland(storage, center, radius, 100);
		
		island->islands[1]->getBoundingBox(&center, &radius);
		int id1 = createIsland(storage, center, radius, 100);
		
		AntQueen* antqueen = storage->antQueen;
		antqueen->position = vec3(2.0f, 1.0f + queenHeightOffset, 2.0f);
		antqueen->goalPoisition = antqueen->position;
		antqueen->radius = 0.5f;
		storage->islands[id0]->antsOnIsland = 50;
		createBridge(storage, id0, id1);
	}

}

int kore(int argc, char** argv) {
	Kore::System::init("Shader", width, height);
	Kore::System::setCallback(update);
	
	startTime = System::time();
	
	Keyboard::the()->KeyDown = keyDown;
	Keyboard::the()->KeyUp = keyUp;

	Mouse::the()->Move = mouseMove;
	Mouse::the()->Press = mousePress;
	Mouse::the()->Release = mouseRelease;
	
#ifdef NDEBUG
	Mouse::the()->lock(0);
#endif
	
	island = new Island();
	
	loadShader();
	// load the skybox and the ground
	skybox = new Skybox();
	skybox->getSkybox(structure);

	planet = new MeshObject("Sphere/sphere.ogex", "Sphere/", structure, 1.0);

	bridge = new MeshObject("AntBridge/AntBridge.ogex", "AntBridge/", structure, 1.0);
	queenTex = new Graphics4::Texture("antQueen.png");
	cameraPos = vec3(-1, 6, -5);

	initWater();
	loadShaderBasicLighting();
	Ant::init();
	Ant::updateDirections();
	
	setUpGameLogic();
	
	font14 = Kravur::load("font/arial", FontStyle(), 14);
	font24 = Kravur::load("font/arial", FontStyle(), 24);
	font34 = Kravur::load("font/arial", FontStyle(), 34);
	font44 = Kravur::load("font/arial", FontStyle(), 44);
	g2 = new Graphics2::Graphics2(width, height);
	g2->setFont(font44);
	
	antTexture = new Graphics4::Texture("ant/ant_tex.png");
	treeTexture = new Graphics4::Texture("island/tree.png");

	Kore::System::start();

	return 0;
}
