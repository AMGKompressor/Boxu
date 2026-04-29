// COMP710 GP Framework 2024

#include "texture.h"

#include "logmanager.h"

#include <SDL.h>
#include <SDL_image.h>
#include <cassert>
#include <cstring>
#include <vector>
#include <GL/glew.h>


Texture::Texture()
	: mTextureId(0)
	, mHeight(0)
	, mWidth(0)
{

}

Texture::~Texture()
{
	if (mTextureId != 0)
	{
		glDeleteTextures(1, &mTextureId);
		mTextureId = 0;
	}
}

bool Texture::initialize(const char* filename)
{
	SDL_Surface* loaded = IMG_Load(filename);
	if (loaded == nullptr)
	{
		LogManager::getInstance().log("Texture failed to load!");
		return false;
	}

	SDL_Surface* surface = SDL_ConvertSurfaceFormat(loaded, SDL_PIXELFORMAT_RGBA32, 0);
	SDL_FreeSurface(loaded);
	if (surface == nullptr)
	{
		LogManager::getInstance().log("Texture format conversion failed!");
		return false;
	}

	mWidth = surface->w;
	mHeight = surface->h;
	const int w = mWidth;
	const int h = mHeight;
	const int pitch = surface->pitch;
	const unsigned char* src = static_cast<const unsigned char*>(surface->pixels);

	// OpenGL's first row is the bottom of the texture; SDL's first row is the top of the image.
	std::vector<unsigned char> rgba(static_cast<std::size_t>(w) * static_cast<std::size_t>(h) * 4u);
	for (int y = 0; y < h; ++y)
	{
		const unsigned char* srcRow = src + static_cast<std::size_t>(y) * static_cast<std::size_t>(pitch);
		unsigned char* dstRow = &rgba[static_cast<std::size_t>(h - 1 - y) * static_cast<std::size_t>(w) * 4u];
		std::memcpy(dstRow, srcRow, static_cast<std::size_t>(w) * 4u);
	}

	SDL_FreeSurface(surface);
	surface = nullptr;

	glGenTextures(1, &mTextureId);
	glBindTexture(GL_TEXTURE_2D, mTextureId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}

bool Texture::initializeFromRgba(int width, int height, const unsigned char* rgbaPixels)
{
	if (width <= 0 || height <= 0 || rgbaPixels == nullptr)
	{
		return false;
	}

	if (mTextureId != 0)
	{
		glDeleteTextures(1, &mTextureId);
		mTextureId = 0;
	}

	mWidth = width;
	mHeight = height;

	glGenTextures(1, &mTextureId);
	glBindTexture(GL_TEXTURE_2D, mTextureId);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA,
		mWidth,
		mHeight,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		rgbaPixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
	return true;
}

void Texture::setActive()
{
	glBindTexture(GL_TEXTURE_2D, mTextureId);
}

int Texture::getWidth() const
{
	assert(mWidth);
	return (mWidth);
}

int Texture::getHeight() const
{
	assert(mHeight);
	return (mHeight);
}
