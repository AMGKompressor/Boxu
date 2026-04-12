// COMP710 GP Framework 2025
#ifndef __SPRITE_H_
#define __SPRITE_H_

class Renderer;
class Texture;

class Sprite
{
public:
	Sprite();
	~Sprite();

	bool initialize(Texture& texture);
	void process(float deltaTime);
	void draw(Renderer& renderer);

	int getWidth() const;
	int getHeight() const;

	void setX(int x);
	int getX() const;
	void setY(int y);
	int getY() const;

	void setAngle(float angle);
	float getAngle() const;

	void setScale(float scale);
	float getScale() const;

	void setAlpha(float alpha);
	float getAlpha() const;

	void setRedTint(float value);
	float getRedTint() const;
	void setGreenTint(float value);
	float getGreenTint() const;
	void setBlueTint(float value);
	float getBlueTint() const;

protected:
	float clamp(float minimum, float value, float maximum);

private:
	Sprite(const Sprite& sprite);
	Sprite& operator=(const Sprite& sprite);

	Texture* mTexture;
	int mX;
	int mY;

	float mAngle;
	int mCenterX;
	int mCenterY;

	int mBaseWidth;
	int mBaseHeight;

	float mScale;
	float mAlpha;

	float mTintRed;
	float mTintGreen;
	float mTintBlue;
};

#endif // __SPRITE_H_
