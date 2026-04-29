// COMP710 GP Framework 2025

#include "texturemanager.h"

#include "texture.h"
#include "logmanager.h"

#include <SDL.h>

TextureManager::TextureManager()
{

}

TextureManager::~TextureManager()
{
	std::map<std::string, Texture*>::iterator iter = mLoadedTextures.begin();

	while (iter != mLoadedTextures.end())
	{
		Texture* texture = iter->second;
		delete texture;

		++iter;
	}

	mLoadedTextures.clear();
}

bool TextureManager::initialize()
{
	LogManager::getInstance().log("TextureManager starting...");

	return true;
}

Texture* TextureManager::getTexture(const char* filename)
{
	Texture* texture = 0;

	if (mLoadedTextures.find(filename) == mLoadedTextures.end())
	{
		texture = new Texture();
		if (!texture->initialize(filename))
		{
			LogManager::getInstance().log("Texture failed to initialize!");
			delete texture;
			return 0;
		}

		mLoadedTextures[filename] = texture;
	}
	else
	{
		texture = mLoadedTextures[filename];
	}

	return texture;
}
