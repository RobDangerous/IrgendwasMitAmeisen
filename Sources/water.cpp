#include "pch.h"

#include "water.h"

#include <Kore/Graphics4/Graphics.h>
#include <Kore/Graphics4/PipelineState.h>
#include <Kore/Graphics4/Texture.h>
#include <Kore/IO/FileReader.h>
#include <Kore/Math/Core.h>
#include <Kore/Math/Random.h>
#include <Kore/System.h>

using namespace Kore;
using namespace Kore::Graphics4;

namespace {
	Texture* vertexMap;
	TextureUnit vertexMapLocation;
	ConstantLocation matrixLocation;
	ConstantLocation vmatrixLocation;
	ConstantLocation timeLocation;
	ConstantLocation zoffsetLocation;
	PipelineState* pipeline;
	VertexBuffer* vertexBuffer;
	IndexBuffer* indexBuffer;
	const int xdiv = 100;
	const int ydiv = 250;
	const int ITER_GEOMETRY = 3;
	const float SEA_CHOPPY = 4.0f;
	const float SEA_SPEED = 0.8f * 5.0f;
	const float SEA_FREQ = 0.16f;
	const float SEA_HEIGHT = 0.6f;
	mat2 octave_m;
}

void initWater() {
	Random::init(0);

	octave_m.Set(0, 0, 1.6f);
	octave_m.Set(0, 1, 1.2f);
	octave_m.Set(1, 0, -1.2f);
	octave_m.Set(1, 1, 1.6f);

	FileReader vs("water.vert");
	FileReader fs("water.frag");
	Shader* vertexShader = new Shader(vs.readAll(), vs.size(), VertexShader);
	Shader* fragmentShader = new Shader(fs.readAll(), fs.size(), FragmentShader);

	vertexMap = new Texture(1024, 1024, Image::Grey8, false);
	u8* pixels = vertexMap->lock();
	for (int y = 0; y < 1024; ++y)
		for (int x = 0; x < 1024; ++x)
			pixels[y * 1024 + x] = Random::get(255);
	vertexMap->unlock();

	VertexStructure structure;
	structure.add("pos", Float2VertexData);
	pipeline = new PipelineState();
	pipeline->inputLayout[0] = &structure;
	pipeline->inputLayout[1] = nullptr;
	pipeline->vertexShader = vertexShader;
	pipeline->fragmentShader = fragmentShader;
	pipeline->depthWrite = true;
	pipeline->depthMode = ZCompareLess;
	pipeline->stencilMode = ZCompareEqual;
	pipeline->stencilWriteMask = 0x00;
	pipeline->stencilReadMask = 0xff;
	pipeline->stencilReferenceValue = 0;
	pipeline->stencilBothPass = Keep;
	pipeline->stencilFail = Keep;
	pipeline->stencilDepthFail = Keep;
	pipeline->compile();

	vertexMapLocation = pipeline->getTextureUnit("tex");
	matrixLocation = pipeline->getConstantLocation("transformation");
	vmatrixLocation = pipeline->getConstantLocation("vtransformation");
	timeLocation = pipeline->getConstantLocation("time");
	zoffsetLocation = pipeline->getConstantLocation("zoffset");

	vertexBuffer = new VertexBuffer(xdiv * ydiv, structure, StaticUsage);
	float* vertices = vertexBuffer->lock();
	float ypos = -1.0;
	float xpos = -1.0;
	for (int y = 0; y < ydiv; ++y) {
		for (int x = 0; x < xdiv; ++x) {
			vertices[y * xdiv * 2 + x * 2 + 0] = (x - (xdiv / 2.0f)) / (xdiv / 2.0f) * 30.0f;
			vertices[y * xdiv * 2 + x * 2 + 1] = (y - (ydiv / 2.0f)) / (ydiv / 2.0f) * 100.0f;
		}
	}
	vertexBuffer->unlock();

	indexBuffer = new IndexBuffer(xdiv * ydiv * 6);
	int* indices = indexBuffer->lock();
	for (int y = 0; y < ydiv - 1; ++y) {
		for (int x = 0; x < xdiv - 1; ++x) {
			indices[y * xdiv * 6 + x * 6 + 0] = y * xdiv + x;
			indices[y * xdiv * 6 + x * 6 + 1] = y * xdiv + x + 1;
			indices[y * xdiv * 6 + x * 6 + 2] = (y + 1) * xdiv + x;
			indices[y * xdiv * 6 + x * 6 + 3] = (y + 1) * xdiv + x;
			indices[y * xdiv * 6 + x * 6 + 4] = y * xdiv + x + 1;
			indices[y * xdiv * 6 + x * 6 + 5] = (y + 1) * xdiv + x + 1;
		}
	}
	indexBuffer->unlock();
}

void renderWater(mat4 matrix, mat4 vmatrix, float zposition) {
	Graphics4::setPipeline(pipeline);
	Graphics4::setFloat(timeLocation, (float)System::time());
	Graphics4::setFloat(zoffsetLocation, zposition - 5.0f);
	Graphics4::setMatrix(matrixLocation, matrix);
	Graphics4::setMatrix(vmatrixLocation, vmatrix);
	Graphics4::setTexture(vertexMapLocation, vertexMap);
	Graphics4::setIndexBuffer(*indexBuffer);
	Graphics4::setVertexBuffer(*vertexBuffer);
	Graphics4::drawIndexedVertices();
}

vec2 sin(vec2 vec) {
	return vec2(Kore::sin(vec.x()), Kore::sin(vec.y()));
}

vec2 cos(vec2 vec) {
	return vec2(Kore::cos(vec.x()), Kore::cos(vec.y()));
}

vec2 abs(vec2 vec) {
	return vec2(Kore::abs(vec.x()), Kore::abs(vec.y()));
}

float fract(float x) {
	return x - Kore::floor(x);
}

vec2 fract(vec2 vec) {
	return vec2(fract(vec.x()), fract(vec.y()));
}

vec2 floor(vec2 vec) {
	return vec2(Kore::floor(vec.x()), Kore::floor(vec.y()));
}

float mix(float x, float y, float a) {
	return x * (1.0f - a) + y * a;
}

vec2 mix(vec2 x, vec2 y, vec2 a) {
	return vec2(mix(x.x(), y.x(), a.x()), mix(x.y(), y.y(), a.y()));
}

vec2 add(vec2 vec, float value) {
	return vec2(vec.x() + value, vec.y() + value);
}

vec2 add(vec2 v1, vec2 v2) {
	return vec2(v1.x() + v2.x(), v1.y() + v2.y());
}

float hash(vec2 p) {
	float h = p.dot(vec2(127.1f, 311.7f));
	return fract(Kore::sin(h) * 43758.5453123f);
}

vec2 mult(vec2 v1, vec2 v2) {
	return vec2(v1.x() * v2.x(), v1.y() * v2.y());
}

float noise(vec2 p) {
	vec2 i = floor(p);
	vec2 f = fract(p);
	vec2 u = mult(f, mult(f, (add(f * -2.0f, 3.0))));
	return -1.0f + 2.0f * mix(mix(hash(add(i, vec2(0.0f, 0.0f))),
		hash(add(i, vec2(1.0, 0.0))), u.x()),
		mix(hash(add(i, vec2(0.0, 1.0))),
			hash(add(i, vec2(1.0, 1.0))), u.x()), u.y());
}

float sea_octave(vec2 uv, float choppy) {
	uv = add(uv, noise(uv));
	vec2 wv = add(abs(sin(uv)) * -1.0f, 1.0f);
	vec2 swv = abs(cos(uv));
	wv = mix(wv, swv, wv);
	return Kore::pow(1.0f - Kore::pow(wv.x() * wv.y(), 0.65f), choppy);
}

float map(vec2 uv) {
	float SEA_TIME = (float)System::time() * SEA_SPEED;

	float freq = SEA_FREQ;
	float amp = SEA_HEIGHT;
	float choppy = SEA_CHOPPY;
	uv.x() *= 0.75f;

	float d = 0.0f;
	float h = 0.0f;
	for (int i = 0; i < 3; ++i) {
		d = sea_octave(add(uv, SEA_TIME) * freq, choppy);
		d += sea_octave(add(uv, -SEA_TIME) * freq, choppy);
		h += d * amp;
		uv = octave_m.Transpose() * uv; freq *= 1.9f; amp *= 0.22f;
		choppy = mix(choppy, 1.0f, 0.2f);
	}
	return h;// p.y - h;
}
