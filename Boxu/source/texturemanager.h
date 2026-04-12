// COMP710 GP Framework 2024
#ifndef __TEXTUREMANAGER_H_
#define __TEXTUREMANAGER_H_

#include <string>
#include <map>

class Texture;

class TextureManager
{
public:
	TextureManager();
	~TextureManager();

	bool initialize();

	Texture* getTexture(const char* filename);

private:
	TextureManager(const TextureManager& textureManager);
	TextureManager& operator=(const TextureManager& textureManager);

	std::map<std::string, Texture*> mLoadedTextures;
};

#endif // __TEXTUREMANAGER_H_
