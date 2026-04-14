// COMP710 GP Framework 2024
#ifndef __TEXTURE_H_
#define __TEXTURE_H_

class Texture
{
public:
	Texture();
	~Texture();

	bool initialize(const char* filename);
	bool initializeFromRgba(int width, int height, const unsigned char* rgbaPixels);

	void setActive();

	int getWidth() const;
	int getHeight() const;

private:
	Texture(const Texture& texture);
	Texture& operator=(const Texture& texture);

	unsigned int mTextureId;
	int mWidth;
	int mHeight;
};

#endif // __TEXTURE_H_
