// COMP710 GP Framework 2024
#ifndef __RENDERER_H_
#define __RENDERER_H_

class TextureManager;
class Texture;
class Shader;
class VertexArray;
class Sprite;
struct SDL_Window;

#include <SDL.h>

class Renderer
{
public:
	Renderer();
	~Renderer();

	bool initialize(bool windowed, int width = 0, int height = 0);

	void clear();
	void present();

	void setClearColor(unsigned char r, unsigned char g, unsigned char b);
	void getClearColor(unsigned char& r, unsigned char& g, unsigned char& b);

	int getWidth() const;
	int getHeight() const;

	void setCamera(float worldOriginX, float worldOriginY);

	Sprite* createSprite(const char* filename);
	void drawSprite(Sprite& sprite);
	void drawWorldAxisAlignedQuad(
		float centerX,
		float centerY,
		float halfWidth,
		float halfHeight,
		float r,
		float g,
		float b,
		float a);

	void drawWorldLineLoop(const float* xyPairs, int vertexCount, float r, float g, float b, float a);
	void drawWorldLineSegments(const float* xyEndpoints, int segmentCount, float r, float g, float b, float a);

	// Fullscreen darkening outside a world-space wedge from (sunekuX, sunekuY) along facingDeg.
	void drawVisionConeMask(
		float sunekuX,
		float sunekuY,
		float facingDeg,
		float halfConeDeg,
		float featherDeg,
		float outsideAlpha,
		float camX,
		float camY,
		const float* wallSegsFlat,
		int wallSegmentCount);

protected:
	bool initializeOpenGL(int screenWidth, int screenHeight);
	void destroyWindowAndContext();
	void setFullscreen(bool fullscreen);

	void logSdlError();

	bool setupSpriteShader();
	bool setupLineDebugGraphics();
	bool setupVisionMaskGraphics();

private:
	Renderer(const Renderer& renderer);
	Renderer& operator=(const Renderer& renderer);

	TextureManager* mTextureManager;
	SDL_Window* mWindow;
	SDL_GLContext mGlContext;

	Shader* mSpriteShader;
	Shader* mLineShader;
	Shader* mVisionMaskShader;
	unsigned int mLineVao;
	unsigned int mLineVbo;
	unsigned int mVisionMaskVao;
	VertexArray* mSpriteVertexData;
	Texture* mSolidWhiteTexture;

	int mWidth;
	int mHeight;

	float mClearRed;
	float mClearGreen;
	float mClearBlue;

	float mCameraX;
	float mCameraY;
};

#endif // __RENDERER_H_
