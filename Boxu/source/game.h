// COMP710 GP Framework
#ifndef __GAME_H_
#define __GAME_H_

#include <array>
#include <cstdint>
#include <random>
#include <vector>

#include <SDL.h>

#include "level_layout.h"

enum class MissionState
{
	Playing,
	Won,
	Lost
};

enum class OcelotState
{
	Dormant,
	Investigate,
	Chase,
	Search
};

class Renderer;
class Sprite;
class Texture;

class PlayerAgent
{
public:
	float x = 0.0f;
	float y = 0.0f;
	float facingDeg = 0.0f;
	float moveSpeed = 130.0f;
};

class EnemyAgent
{
public:
	float x = 0.0f;
	float y = 0.0f;
	float facingDeg = 180.0f;
	float investigateX = 0.0f;
	float investigateY = 0.0f;
	float lastSeenX = 0.0f;
	float lastSeenY = 0.0f;
	float searchTimer = 0.0f;
	float hearingRadius = 320.0f;
	float visionRange = 520.0f;
	float visionHalfAngleDeg = 40.0f;
	float chaseSpeed = 108.0f;
	float investigateSpeed = 90.0f;
	float searchSpeed = 82.0f;
	bool awake = true;
	OcelotState state = OcelotState::Investigate;
	int patrolIndex = 0;
};

class Game
{
public:
	static Game& getInstance();
	static void destroyInstance();
	bool initialize();
	bool doGameLoop();
	void quit();

	static constexpr float kTutorialEntryWestX = Layout::kEntryWest;
	static constexpr float kTutorialEntryEastX = Layout::kEntryEast;

protected:
	void process(float deltaTime);
	void draw(Renderer& renderer);
	void processFrameCounting(float deltaTime);
	void updateCamera();
	void updateSunekuFacingTowardMouse(float deltaTime);
	void constrainSunekuToTutorialOutline(
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
	void freeFootstepClips();
	void tryLoadFootstepClips(const SDL_AudioSpec& deviceHave);
	void playNextFootstepClip();
	void emitNoiseEvent(float x, float y, bool loud);
	void resetMission();
	void updateOcelot(float deltaTime);
	bool hasWallOcclusion(float ax, float ay, float bx, float by) const;
	bool pickOcelotSteerTarget(float goalX, float goalY, float& outX, float& outY) const;
	bool pickEnemySteerTarget(const EnemyAgent& enemy, float goalX, float goalY, float& outX, float& outY) const;
	void updateEnemyAgent(EnemyAgent& enemy, int enemyIndex, float deltaTime);
	void resetExtraEnemies();
	const float* activeWireFlat() const;
	int activeWireSegmentCount() const;
	void setupLevel(int index);
	void updateSplashIntro(float deltaTime);
	void drawSplashIntro(Renderer& renderer);
	void layoutSplashSpriteCenter(Sprite* sprite, Renderer& renderer);

private:
	Game();
	~Game();
	Game(const Game& game);
	Game& operator=(const Game& game);

	static Game* sInstance;
	Renderer* mRenderer;
	Sprite* mFloor;
	Sprite* mSuneku;
	Sprite* mSunekuHitboxDebug;
	Texture* mVersionTexture;
	Sprite* mVersionSprite;
	Texture* mHudObjectiveTexture;
	Sprite* mHudObjectiveSprite;
	Texture* mHudWinTexture;
	Sprite* mHudWinSprite;
	Texture* mHudLoseTexture;
	Sprite* mHudLoseSprite;
	float mSunekuX;
	float mSunekuY;
	float mSunekuHitboxHalfW;
	float mSunekuHitboxHalfH;
	float mMapWidth;
	float mMapHeight;
	float mCameraX;
	float mCameraY;
	float mSunekuFacingDeg;
	float mSunekuMoveSpeed;
	bool mShowSunekuHitbox;

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
	std::uint32_t mFootstepAudioDevice;
	unsigned char* mWallHitPcm;
	std::uint32_t mWallHitPcmLen;
	bool mWallHitPcmFromLoadWav;

	static constexpr int kFootstepClipCount = 4;
	unsigned char* mFootstepPcm[kFootstepClipCount];
	std::uint32_t mFootstepPcmLen[kFootstepClipCount];
	bool mFootstepPcmFromLoadWav[kFootstepClipCount];
	bool mFootstepClipsReady;
	float mFootstepCooldown;
	int mFootstepShuffleIndex;
	std::array<int, kFootstepClipCount> mFootstepShuffle;

	std::vector<bool> mOutlineWallPrevTouch;

	float mLastWallHitNoiseRadius;
	float mWallNoisePulseAgeWalk;
	float mWallNoisePulseMaxRWalk;
	float mWallNoisePulseCxWalk;
	float mWallNoisePulseCyWalk;
	bool mWallNoisePulseActiveWalk;
	float mWallNoisePulseAgeSprint;
	float mWallNoisePulseMaxRSprint;
	float mWallNoisePulseCxSprint;
	float mWallNoisePulseCySprint;
	bool mWallNoisePulseActiveSprint;

	float mOcelotX;
	float mOcelotY;
	float mOcelotFacingDeg;
	float mOcelotInvestigateX;
	float mOcelotInvestigateY;
	float mOcelotLastSeenX;
	float mOcelotLastSeenY;
	float mOcelotSearchTimer;
	float mOcelotHearingRadius;
	int mOcelotPatrolIndex;
	bool mOcelotAwake;
	OcelotState mOcelotState;
	int mCurrentLevel;

	MissionState mMissionState;
	bool mHasKeyCard;
	float mKeyCardX;
	float mKeyCardY;
	float mExtractCenterX;
	float mExtractCenterY;
	float mExtractHalfW;
	float mExtractHalfH;
	PlayerAgent mPlayerAgent;
	std::vector<EnemyAgent> mExtraEnemies;
	std::mt19937 mRng;

	Sprite* mSplashAutSprite;
	Sprite* mSplashBoxuSprite;
	bool mSplashSequenceActive;
	float mSplashElapsed;
};

#endif // __GAME_H_
