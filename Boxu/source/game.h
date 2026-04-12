// COMP710 GP Framework
#ifndef __GAME_H_
#define __GAME_H_

#include <array>
#include <cstdint>

#include "level_layout.h"

class Renderer;
class Sprite;

class Game
{
public:
	static Game& getInstance();
	static void destroyInstance();
	bool initialize();
	bool doGameLoop();
	void quit();

	static constexpr int kTutorialWallPhysicsSegmentCount = Layout::kWallCount;
	static constexpr float kTutorialEntryWestX = Layout::kEntryWest;
	static constexpr float kTutorialEntryEastX = Layout::kEntryEast;

protected:
	void process(float deltaTime);
	void draw(Renderer& renderer);
	void processFrameCounting(float deltaTime);
	void updateCamera();
	void updateFacingTowardMouse(float deltaTime);
	void constrainPlayerToTutorialOutline(
		float& cx,
		float& cy,
		float worldHalfX,
		float worldHalfY,
		float localHalfW,
		float localHalfH,
		float facingAngleDeg);
	bool tryInitWallHitAudio();
	void shutdownWallHitAudio();
	void playWallHitSoundIfReady();

private:
	Game();
	~Game();
	Game(const Game& game);
	Game& operator=(const Game& game);

	static Game* sInstance;
	Renderer* mRenderer;
	Sprite* mFloor;
	Sprite* mPlayer;
	Sprite* mPlayerHitboxDebug;
	float mPlayerX;
	float mPlayerY;
	float mPlayerHitboxHalfW;
	float mPlayerHitboxHalfH;
	float mMapWidth;
	float mMapHeight;
	float mCameraX;
	float mCameraY;
	float mFacingAngleDeg;
	bool mShowDebugHitbox;

	std::int64_t mLastTime;
	float mExecutionTime;
	float mElapsedSeconds;
	int mFrameCount;
	int mFps;

#ifdef USE_LAG
	float mLag;
	int mUpdateCount;
#endif

	bool mLooping;

	std::uint32_t mWallHitAudioDevice;
	unsigned char* mWallHitPcm;
	std::uint32_t mWallHitPcmLen;
	bool mWallHitPcmFromLoadWav;

	std::array<bool, kTutorialWallPhysicsSegmentCount> mOutlineWallPrevTouch;
};

#endif // __GAME_H_
