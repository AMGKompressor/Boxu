// COMP710 GP Framework 2024

#include "texture.h"

#include "logmanager.h"

#include <SDL_image.h>
#include <cassert>
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
	SDL_Surface* surface = IMG_Load(filename);

	if (surface)
	{
		mWidth = surface->w;
		mHeight = surface->h;

		int bytesPerPixel = surface->format->BytesPerPixel;

		unsigned int format = 0;

		if (bytesPerPixel == 3)
		{
			format = GL_RGB;
		}
		else if (bytesPerPixel == 4)
		{
			format = GL_RGBA;
		}

		glGenTextures(1, &mTextureId);
		glBindTexture(GL_TEXTURE_2D, mTextureId);

		glTexImage2D(GL_TEXTURE_2D, 0, format, mWidth, mHeight, 0, format, GL_UNSIGNED_BYTE, surface->pixels);

		SDL_FreeSurface(surface);
		surface = 0;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	else
	{
		LogManager::getInstance().log("Texture failed to load!");
		assert(0);
		return false;
	}

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
