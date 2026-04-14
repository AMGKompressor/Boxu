// COMP710 GP Framework 2025

#include "renderer.h"

#include "texturemanager.h"
#include "texture.h"
#include "logmanager.h"
#include "shader.h"
#include "vertexarray.h"
#include "sprite.h"
#include "matrix4.h"

#include <SDL.h>
#include <SDL_filesystem.h>
#include <SDL_image.h>

#include <algorithm>
#include <string>
#include <GL/glew.h>
#include <cassert>
#include <cmath>

Renderer::Renderer()
	: mTextureManager(0)
	, mSpriteShader(0)
	, mLineShader(0)
	, mVisionMaskShader(0)
	, mLineVao(0)
	, mLineVbo(0)
	, mVisionMaskVao(0)
	, mSpriteVertexData(0)
	, mSolidWhiteTexture(0)
	, mGlContext(0)
	, mWidth(0)
	, mHeight(0)
	, mClearRed(0.0f)
	, mClearGreen(0.0f)
	, mClearBlue(0.0f)
	, mCameraX(0.0f)
	, mCameraY(0.0f)
{

}

Renderer::~Renderer()
{
	if (mLineVao != 0)
	{
		glDeleteVertexArrays(1, &mLineVao);
		mLineVao = 0;
	}
	if (mLineVbo != 0)
	{
		glDeleteBuffers(1, &mLineVbo);
		mLineVbo = 0;
	}
	if (mVisionMaskVao != 0)
	{
		glDeleteVertexArrays(1, &mVisionMaskVao);
		mVisionMaskVao = 0;
	}
	delete mLineShader;
	mLineShader = 0;
	delete mVisionMaskShader;
	mVisionMaskShader = 0;

	delete mSpriteShader;
	mSpriteShader = 0;

	delete mSolidWhiteTexture;
	mSolidWhiteTexture = 0;

	delete mSpriteVertexData;
	mSpriteVertexData = 0;

	delete mTextureManager;
	mTextureManager = 0;

	SDL_DestroyWindow(mWindow);
	IMG_Quit();
	SDL_Quit();
}

bool Renderer::initialize(bool windowed, int width, int height)
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		logSdlError();
		return false;
	}

	char* shaderBase = SDL_GetBasePath();
	if (shaderBase != nullptr)
	{
		std::string root(shaderBase);
		SDL_free(shaderBase);
		if (!root.empty())
		{
			const char last = root.back();
			if (last != '/' && last != '\\')
			{
				root.push_back('/');
			}
		}
		Shader::setShaderSearchRoot(root);
	}
	else
	{
		Shader::setShaderSearchRoot(std::string());
	}

	if (!windowed)
	{
		int numDisplays = SDL_GetNumVideoDisplays();
		SDL_DisplayMode* currentDisplayMode = new SDL_DisplayMode[numDisplays];

		for (int k = 0; k < numDisplays; ++k)
		{
			SDL_GetCurrentDisplayMode(k, &currentDisplayMode[k]);
		}

		int widest = 0;
		int andItsHeight = 0;

		for (int k = 0; k < numDisplays; ++k)
		{
			if (currentDisplayMode[k].w > widest)
			{
				widest = currentDisplayMode[k].w;
				andItsHeight = currentDisplayMode[k].h;
			}
		}

		delete[] currentDisplayMode;
		currentDisplayMode = 0;

		width = widest;
		height = andItsHeight;
	}

	bool initialized = initializeOpenGL(width, height);

	setFullscreen(!windowed);

	if (initialized)
	{
		mTextureManager = new TextureManager();
		assert(mTextureManager);
		initialized = mTextureManager->initialize();
	}

	if (initialized)
	{
		mSolidWhiteTexture = new Texture();
		const unsigned char px[4] = {255, 255, 255, 255};
		if (!mSolidWhiteTexture->initializeFromRgba(1, 1, px))
		{
			delete mSolidWhiteTexture;
			mSolidWhiteTexture = 0;
		}
	}

	return initialized;
}

bool Renderer::initializeOpenGL(int screenWidth, int screenHeight)
{
	mWidth = screenWidth;
	mHeight = screenHeight;

	mWindow = SDL_CreateWindow("COMP710 Game Framework 2025", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenWidth, screenHeight, SDL_WINDOW_OPENGL);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

	mGlContext = SDL_GL_CreateContext(mWindow);

	glewExperimental = GL_TRUE;
	GLenum glewResult = glewInit();

	if (glewResult != GLEW_OK)
	{
		return false;
	}

	SDL_GL_SetSwapInterval(0);

	bool shadersLoaded = setupSpriteShader();
	if (shadersLoaded)
	{
		if (!setupLineDebugGraphics())
		{
			LogManager::getInstance().log("Line debug shader failed (level outline will not draw).");
		}
		if (!setupVisionMaskGraphics())
		{
			LogManager::getInstance().log("Vision mask shader failed (cone vision disabled).");
		}
	}

	return shadersLoaded;
}

void Renderer::clear()
{
	glClearColor(mClearRed, mClearGreen, mClearBlue, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::present()
{
	SDL_GL_SwapWindow(mWindow);
}

void Renderer::setFullscreen(bool fullscreen)
{
	if (fullscreen)
	{
		SDL_SetWindowFullscreen(mWindow, SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_ALWAYS_ON_TOP);
		SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");
		SDL_SetWindowSize(mWindow, mWidth, mHeight);
	}
	else
	{
		SDL_SetWindowFullscreen(mWindow, 0);
	}
}

void Renderer::setClearColor(unsigned char r, unsigned char g, unsigned char b)
{
	mClearRed = r / 255.0f;
	mClearGreen = g / 255.0f;
	mClearBlue = b / 255.0f;
}

void Renderer::getClearColor(unsigned char& r, unsigned char& g, unsigned char& b)
{
	r = static_cast<unsigned char>(mClearRed * 255.0f);
	g = static_cast<unsigned char>(mClearGreen * 255.0f);
	b = static_cast<unsigned char>(mClearBlue * 255.0f);
}

int Renderer::getWidth() const
{
	return mWidth;
}

int Renderer::getHeight() const
{
	return mHeight;
}

void Renderer::setCamera(float worldOriginX, float worldOriginY)
{
	mCameraX = worldOriginX;
	mCameraY = worldOriginY;
}

Sprite* Renderer::createSprite(const char* filename)
{
	assert(mTextureManager);

	Texture* texture = mTextureManager->getTexture(filename);

	Sprite* sprite = new Sprite();
	if (!sprite->initialize(*texture))
	{
		LogManager::getInstance().log("Sprite Failed to Create!");
	}

	return sprite;
}

void Renderer::logSdlError()
{
	LogManager::getInstance().log(SDL_GetError());
}

bool Renderer::setupSpriteShader()
{
	mSpriteShader = new Shader();

	bool loaded = mSpriteShader->load("shaders/sprite.vert", "shaders/sprite.frag");

	mSpriteShader->setActive();

	float vertices[] =
	{
		-0.5f,  0.5f, 0.0f, 0.0f, 0.0f,
		 0.5f,  0.5f, 0.0f, 1.0f, 0.0f,
		 0.5f, -0.5f, 0.0f, 1.0f, 1.0f,
		-0.5f, -0.5f, 0.0f, 0.0f, 1.0f
	};

	unsigned int indices[] = { 0,1,2,2,3,0 };

	mSpriteVertexData = new VertexArray(vertices, 4, indices, 6);

	return loaded;
}

void Renderer::drawSprite(Sprite& sprite)
{
	mSpriteShader->setActive();
	mSpriteVertexData->setActive();

	float angleInDegrees = sprite.getAngle();

	float sizeX = static_cast<float>(sprite.getWidth());
	float sizeY = static_cast<float>(sprite.getHeight());

	const float PI = 3.14159f;
	float angleInRadians = (angleInDegrees * PI) / 180.0f;

	Matrix4 world;
	setIdentity(world);
	world.m[0][0] = cosf(angleInRadians) * (sizeX);
	world.m[0][1] = -sinf(angleInRadians) * (sizeX);
	world.m[1][0] = sinf(angleInRadians) * (sizeY);
	world.m[1][1] = cosf(angleInRadians) * (sizeY);
	world.m[3][0] = static_cast<float>(sprite.getX());
	world.m[3][1] = static_cast<float>(sprite.getY());

	mSpriteShader->setMatrixUniform("uWorldTransform", world);

	Matrix4 ortho;
	createOrthoProjection(ortho, static_cast<float>(mWidth), static_cast<float>(mHeight));

	Matrix4 cam;
	createTranslation(cam, -mCameraX, -mCameraY);

	Matrix4 orthoViewProj;
	matrixMultiply(orthoViewProj, cam, ortho);

	mSpriteShader->setVector4Uniform("color", sprite.getRedTint(), sprite.getGreenTint(), sprite.getBlueTint(), sprite.getAlpha());

	mSpriteShader->setMatrixUniform("uViewProj", orthoViewProj);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void Renderer::drawWorldAxisAlignedQuad(
	float centerX,
	float centerY,
	float halfWidth,
	float halfHeight,
	float r,
	float g,
	float b,
	float a)
{
	if (mSpriteShader == 0 || mSpriteVertexData == 0 || mSolidWhiteTexture == 0)
	{
		return;
	}
	if (halfWidth <= 0.0f || halfHeight <= 0.0f)
	{
		return;
	}

	mSolidWhiteTexture->setActive();
	mSpriteShader->setActive();
	mSpriteVertexData->setActive();

	Matrix4 world;
	setIdentity(world);
	world.m[0][0] = halfWidth * 2.0f;
	world.m[1][1] = halfHeight * 2.0f;
	world.m[3][0] = centerX;
	world.m[3][1] = centerY;

	mSpriteShader->setMatrixUniform("uWorldTransform", world);

	Matrix4 ortho;
	createOrthoProjection(ortho, static_cast<float>(mWidth), static_cast<float>(mHeight));

	Matrix4 cam;
	createTranslation(cam, -mCameraX, -mCameraY);

	Matrix4 orthoViewProj;
	matrixMultiply(orthoViewProj, cam, ortho);

	mSpriteShader->setVector4Uniform("color", r, g, b, a);
	mSpriteShader->setMatrixUniform("uViewProj", orthoViewProj);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

bool Renderer::setupLineDebugGraphics()
{
	mLineShader = new Shader();
	if (!mLineShader->load("shaders/line.vert", "shaders/line.frag"))
	{
		delete mLineShader;
		mLineShader = 0;
		return false;
	}

	const int kMaxLineVerts = 64;
	glGenVertexArrays(1, &mLineVao);
	glGenBuffers(1, &mLineVbo);
	glBindVertexArray(mLineVao);
	glBindBuffer(GL_ARRAY_BUFFER, mLineVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * kMaxLineVerts, nullptr, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return true;
}

void Renderer::drawWorldLineLoop(const float* xyPairs, int vertexCount, float r, float g, float b, float a)
{
	if (mLineShader == 0 || mLineVao == 0 || mLineVbo == 0 || xyPairs == 0 || vertexCount < 3)
	{
		return;
	}

	float interleaved[64 * 3];
	const int cap = 64;
	if (vertexCount > cap)
	{
		vertexCount = cap;
	}
	for (int i = 0; i < vertexCount; ++i)
	{
		interleaved[i * 3 + 0] = xyPairs[i * 2 + 0];
		interleaved[i * 3 + 1] = xyPairs[i * 2 + 1];
		interleaved[i * 3 + 2] = 0.0f;
	}

	mLineShader->setActive();
	glBindVertexArray(mLineVao);
	glBindBuffer(GL_ARRAY_BUFFER, mLineVbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * 3 * vertexCount, interleaved);

	Matrix4 ortho;
	createOrthoProjection(ortho, static_cast<float>(mWidth), static_cast<float>(mHeight));

	Matrix4 cam;
	createTranslation(cam, -mCameraX, -mCameraY);

	Matrix4 orthoViewProj;
	matrixMultiply(orthoViewProj, cam, ortho);

	mLineShader->setMatrixUniform("uViewProj", orthoViewProj);
	mLineShader->setVector4Uniform("uColor", r, g, b, a);

	glBindTexture(GL_TEXTURE_2D, 0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glLineWidth(2.0f);
	glDrawArrays(GL_LINE_LOOP, 0, vertexCount);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Renderer::drawWorldLineSegments(const float* xyEndpoints, int segmentCount, float r, float g, float b, float a)
{
	if (mLineShader == 0 || mLineVao == 0 || mLineVbo == 0 || xyEndpoints == 0 || segmentCount < 1)
	{
		return;
	}

	const int capVerts = 128;
	int seg = segmentCount;
	int vertCount = seg * 2;
	if (vertCount > capVerts)
	{
		seg = capVerts / 2;
		vertCount = seg * 2;
	}

	float interleaved[128 * 3];
	for (int s = 0; s < seg; ++s)
	{
		const int o = s * 4;
		interleaved[s * 6 + 0] = xyEndpoints[o + 0];
		interleaved[s * 6 + 1] = xyEndpoints[o + 1];
		interleaved[s * 6 + 2] = 0.0f;
		interleaved[s * 6 + 3] = xyEndpoints[o + 2];
		interleaved[s * 6 + 4] = xyEndpoints[o + 3];
		interleaved[s * 6 + 5] = 0.0f;
	}

	mLineShader->setActive();
	glBindVertexArray(mLineVao);
	glBindBuffer(GL_ARRAY_BUFFER, mLineVbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * 3 * vertCount, interleaved);

	Matrix4 ortho;
	createOrthoProjection(ortho, static_cast<float>(mWidth), static_cast<float>(mHeight));

	Matrix4 cam;
	createTranslation(cam, -mCameraX, -mCameraY);

	Matrix4 orthoViewProj;
	matrixMultiply(orthoViewProj, cam, ortho);

	mLineShader->setMatrixUniform("uViewProj", orthoViewProj);
	mLineShader->setVector4Uniform("uColor", r, g, b, a);

	glBindTexture(GL_TEXTURE_2D, 0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glLineWidth(2.0f);

	glDrawArrays(GL_LINES, 0, vertCount);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

bool Renderer::setupVisionMaskGraphics()
{
	mVisionMaskShader = new Shader();
	if (!mVisionMaskShader->load("shaders/vision_mask.vert", "shaders/vision_mask.frag"))
	{
		delete mVisionMaskShader;
		mVisionMaskShader = 0;
		return false;
	}

	glGenVertexArrays(1, &mVisionMaskVao);
	return true;
}

void Renderer::drawVisionConeMask(
	float sunekuX,
	float sunekuY,
	float facingDeg,
	float halfConeDeg,
	float featherDeg,
	float outsideAlpha,
	float camX,
	float camY,
	const float* wallSegsFlat,
	int wallSegmentCount)
{
	if (mVisionMaskShader == 0 || mVisionMaskVao == 0)
	{
		return;
	}

	const float degToRad = 3.14159265f / 180.0f;
	const float halfConeRad = halfConeDeg * degToRad;
	const float featherRad = featherDeg * degToRad;

	mVisionMaskShader->setActive();
	glBindVertexArray(mVisionMaskVao);

	mVisionMaskShader->setFloatUniform("uSunekuX", sunekuX);
	mVisionMaskShader->setFloatUniform("uSunekuY", sunekuY);
	mVisionMaskShader->setFloatUniform("uFacingDeg", facingDeg);
	mVisionMaskShader->setFloatUniform("uHalfConeRad", halfConeRad);
	mVisionMaskShader->setFloatUniform("uFeatherRad", featherRad);
	mVisionMaskShader->setFloatUniform("uOutsideAlpha", outsideAlpha);
	mVisionMaskShader->setFloatUniform("uCamX", camX);
	mVisionMaskShader->setFloatUniform("uCamY", camY);
	mVisionMaskShader->setFloatUniform("uViewW", static_cast<float>(mWidth));
	mVisionMaskShader->setFloatUniform("uViewH", static_cast<float>(mHeight));

	const int kMaxVisionWalls = 32;
	int nWall = 0;
	if (wallSegsFlat != nullptr && wallSegmentCount > 0)
	{
		nWall = std::min(wallSegmentCount, kMaxVisionWalls);
		mVisionMaskShader->setVec4ArrayUniform("uWallSeg", wallSegsFlat, nWall);
	}
	mVisionMaskShader->setIntUniform("uWallSegCount", nWall);

	glBindTexture(GL_TEXTURE_2D, 0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	glBindVertexArray(0);
}
