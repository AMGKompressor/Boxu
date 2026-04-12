// COMP710 GP Framework

#include "game.h"

#include <SDL.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstring>

#include "renderer.h"
#include "logmanager.h"
#include "sprite.h"

namespace
{
	// AABB half-extents for a centered rect (halfW, halfH) rotated by angleDeg (Sprite convention).
	void orientedRectWorldAabbHalfExtents(float halfW, float halfH, float angleDeg, float& outHalfX, float& outHalfY)
	{
		const float rad = angleDeg * 3.14159265f / 180.0f;
		const float c = std::fabs(std::cos(rad));
		const float s = std::fabs(std::sin(rad));
		outHalfX = c * halfW + s * halfH;
		outHalfY = s * halfW + c * halfH;
	}

	template <std::size_t N>
	constexpr std::array<float, N * 4u> flattenLayoutWalls(const std::array<Layout::Wall, N>& walls)
	{
		std::array<float, N * 4u> out{};
		for (std::size_t i = 0; i < N; ++i)
		{
			out[i * 4u + 0u] = walls[i].x0;
			out[i * 4u + 1u] = walls[i].y0;
			out[i * 4u + 2u] = walls[i].x1;
			out[i * 4u + 3u] = walls[i].y1;
		}
		return out;
	}

	static constexpr auto kTutorialWireFlat = flattenLayoutWalls(Layout::kWalls);
	static constexpr int kTutorialWireSegmentCount = Layout::kWallCount;
	static_assert(
		static_cast<int>(kTutorialWireFlat.size()) == kTutorialWireSegmentCount * 4,
		"wall flat buffer must hold 4 floats per segment");
	static_assert(kTutorialWireSegmentCount == Game::kTutorialWallPhysicsSegmentCount, "layout vs game.h wall count");

	// OBB half-extent along inward normal N (same basis as Renderer::drawSprite).
	float obbExtentAlongInwardNormal(float nx, float ny, float localHalfW, float localHalfH, float angleDeg)
	{
		const float rad = angleDeg * 3.14159265f / 180.0f;
		const float c = std::cos(rad);
		const float s = std::sin(rad);
		const float dotW = c * nx - s * ny;
		const float dotH = s * nx + c * ny;
		return localHalfW * std::fabs(dotW) + localHalfH * std::fabs(dotH);
	}

	void closestPointOnSegment2D(
		float px,
		float py,
		float ax,
		float ay,
		float bx,
		float by,
		float& outQx,
		float& outQy)
	{
		const float abx = bx - ax;
		const float aby = by - ay;
		const float apx = px - ax;
		const float apy = py - ay;
		const float ab2 = abx * abx + aby * aby;
		if (ab2 < 1.0e-12f)
		{
			outQx = ax;
			outQy = ay;
			return;
		}
		float t = (apx * abx + apy * aby) / ab2;
		if (t < 0.0f)
		{
			t = 0.0f;
		}
		else if (t > 1.0f)
		{
			t = 1.0f;
		}
		outQx = ax + t * abx;
		outQy = ay + t * aby;
	}

	void constrainCenterToWireSegments(
		float& px,
		float& py,
		const float* segXY,
		int segmentCount,
		float localHalfW,
		float localHalfH,
		float angleDeg)
	{
		if (segXY == 0 || segmentCount < 1)
		{
			return;
		}
		constexpr int kMaxIters = 32;
		for (int iter = 0; iter < kMaxIters; ++iter)
		{
			float worstMargin = 1.0e30f;
			float pushNx = 0.0f;
			float pushNy = 0.0f;

			for (int s = 0; s < segmentCount; ++s)
			{
				const int o = s * 4;
				const float ax = segXY[o + 0];
				const float ay = segXY[o + 1];
				const float bx = segXY[o + 2];
				const float by = segXY[o + 3];

				float qx = 0.0f;
				float qy = 0.0f;
				closestPointOnSegment2D(px, py, ax, ay, bx, by, qx, qy);

				float mx = px - qx;
				float my = py - qy;
				float dist = std::sqrt(mx * mx + my * my);
				float ndx = 0.0f;
				float ndy = 0.0f;
				if (dist < 1.0e-4f)
				{
					const float abx = bx - ax;
					const float aby = by - ay;
					const float ablen = std::sqrt(abx * abx + aby * aby);
					if (ablen < 1.0e-6f)
					{
						continue;
					}
					ndx = -aby / ablen;
					ndy = abx / ablen;
					dist = 0.0f;
				}
				else
				{
					ndx = mx / dist;
					ndy = my / dist;
				}

				const float req = obbExtentAlongInwardNormal(ndx, ndy, localHalfW, localHalfH, angleDeg);
				const float margin = dist - req;
				if (margin < worstMargin)
				{
					worstMargin = margin;
					pushNx = ndx;
					pushNy = ndy;
				}
			}

			if (worstMargin >= -0.01f)
			{
				break;
			}
			px += pushNx * (-worstMargin);
			py += pushNy * (-worstMargin);
		}
	}
}

Game* Game::sInstance = 0;

Game& Game::getInstance()
{
	if (sInstance == 0)
	{
		sInstance = new Game();
	}
	return (*sInstance);
}

void Game::destroyInstance()
{
	delete sInstance;
	sInstance = 0;
}

Game::Game()
	: mRenderer(0)
	, mFloor(0)
	, mPlayer(0)
	, mPlayerHitboxDebug(0)
	, mPlayerX(0.0f)
	, mPlayerY(0.0f)
	, mPlayerHitboxHalfW(48.0f)
	, mPlayerHitboxHalfH(48.0f)
	, mMapWidth(2560.0f)
	, mMapHeight(1920.0f)
	, mCameraX(0.0f)
	, mCameraY(0.0f)
	, mFacingAngleDeg(0.0f)
	, mShowDebugHitbox(true)
	, mLastTime(0)
	, mExecutionTime(0.0f)
	, mElapsedSeconds(0.0f)
	, mFrameCount(0)
	, mFps(0)
#ifdef USE_LAG
	, mLag(0.0f)
	, mUpdateCount(0)
#endif
	, mLooping(true)
	, mWallHitAudioDevice(0)
	, mWallHitPcm(0)
	, mWallHitPcmLen(0)
	, mWallHitPcmFromLoadWav(false)
	, mOutlineWallPrevTouch{}
{
}

Game::~Game()
{
	shutdownWallHitAudio();

	delete mPlayerHitboxDebug;
	mPlayerHitboxDebug = 0;

	delete mPlayer;
	mPlayer = 0;

	delete mFloor;
	mFloor = 0;

	delete mRenderer;
	mRenderer = 0;
}

void Game::quit()
{
	mLooping = false;
}

bool Game::initialize()
{
	int bbWidth = 1024;
	int bbHeight = 768;

	mRenderer = new Renderer();
	if (!mRenderer->initialize(true, bbWidth, bbHeight))
	{
		LogManager::getInstance().log("Renderer failed to initialize!");
		return false;
	}

	bbWidth = mRenderer->getWidth();
	bbHeight = mRenderer->getHeight();

	mLastTime = SDL_GetPerformanceCounter();

	mRenderer->setClearColor(0, 0, 0);

	mFloor = 0;

	mPlayer = mRenderer->createSprite("textures/board8x8.png");
	if (mPlayer != 0)
	{
		const float desiredPlayerSize = 96.0f;
		const float playerScale = desiredPlayerSize / static_cast<float>(mPlayer->getWidth());
		mPlayer->setScale(playerScale);
		mPlayer->setRedTint(0.55f);
		mPlayer->setGreenTint(0.35f);
		mPlayer->setBlueTint(0.2f);

		mPlayerHitboxHalfW = static_cast<float>(mPlayer->getWidth()) * 0.5f;
		mPlayerHitboxHalfH = static_cast<float>(mPlayer->getHeight()) * 0.5f;

		mPlayerHitboxDebug = mRenderer->createSprite("textures/board8x8.png");
		if (mPlayerHitboxDebug != 0)
		{
			mPlayerHitboxDebug->setScale(playerScale);
			mPlayerHitboxDebug->setRedTint(1.0f);
			mPlayerHitboxDebug->setGreenTint(0.0f);
			mPlayerHitboxDebug->setBlueTint(0.0f);
			mPlayerHitboxDebug->setAlpha(0.3f);
		}

		mPlayerX = (Game::kTutorialEntryWestX + Game::kTutorialEntryEastX) * 0.5f;
		mPlayerY = 900.0f;
		mPlayer->setX(static_cast<int>(mPlayerX));
		mPlayer->setY(static_cast<int>(mPlayerY));
		if (mPlayerHitboxDebug != 0)
		{
			mPlayerHitboxDebug->setX(static_cast<int>(mPlayerX));
			mPlayerHitboxDebug->setY(static_cast<int>(mPlayerY));
		}
		updateCamera();
		mRenderer->setCamera(mCameraX, mCameraY);
	}
	else
	{
		LogManager::getInstance().log("Failed to create player sprite.");
	}

	if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
	{
		LogManager::getInstance().log("SDL audio unavailable (continuing without wall-hit sound).");
	}
	else if (!tryInitWallHitAudio())
	{
		LogManager::getInstance().log("Wall-hit sound not loaded (sounds/thud.wav).");
	}

	return true;
}

bool Game::doGameLoop()
{
	const float stepSize = 1.0f / 60.0f;

	SDL_Event event;
	while (SDL_PollEvent(&event) != 0)
	{
		if (event.type == SDL_QUIT)
		{
			quit();
		}
		else if (event.type == SDL_KEYDOWN && event.key.repeat == 0)
		{
			if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
			{
				quit();
			}
			else if (event.key.keysym.scancode == SDL_SCANCODE_H)
			{
				mShowDebugHitbox = !mShowDebugHitbox;
			}
		}
	}

	if (mLooping)
	{
		Uint64 current = SDL_GetPerformanceCounter();
		float deltaTime = (current - mLastTime) / static_cast<float>(SDL_GetPerformanceFrequency());

		mLastTime = current;

		mExecutionTime += deltaTime;

		process(deltaTime);

#ifdef USE_LAG
		mLag += deltaTime;

		int innerLag = 0;

		while (mLag >= stepSize)
		{
			process(stepSize);

			mLag -= stepSize;

			++mUpdateCount;
			++innerLag;
		}
#endif

		draw(*mRenderer);
	}

	return mLooping;
}

void Game::process(float deltaTime)
{
	if (deltaTime < 0.0f)
	{
		deltaTime = 0.0f;
	}
	else if (deltaTime > 0.1f)
	{
		deltaTime = 0.1f;
	}

	processFrameCounting(deltaTime);

	if (mPlayer != 0)
	{
		SDL_PumpEvents();
		updateFacingTowardMouse(deltaTime);
		mPlayer->setAngle(mFacingAngleDeg);
		if (mPlayerHitboxDebug != 0)
		{
			mPlayerHitboxDebug->setAngle(mFacingAngleDeg);
		}

		const float speed = 250.0f;
		const Uint8* keys = SDL_GetKeyboardState(0);

		float dx = 0.0f;
		float dy = 0.0f;
		if (keys[SDL_SCANCODE_W]) { dy -= 1.0f; }
		if (keys[SDL_SCANCODE_S]) { dy += 1.0f; }
		if (keys[SDL_SCANCODE_A]) { dx -= 1.0f; }
		if (keys[SDL_SCANCODE_D]) { dx += 1.0f; }

		if (dx != 0.0f && dy != 0.0f)
		{
			const float invLen = 0.70710678f;
			dx *= invLen;
			dy *= invLen;
		}

		const float nextX = mPlayerX + dx * speed * deltaTime;
		const float nextY = mPlayerY + dy * speed * deltaTime;

		float worldHalfX = 0.0f;
		float worldHalfY = 0.0f;
		orientedRectWorldAabbHalfExtents(
			mPlayerHitboxHalfW, mPlayerHitboxHalfH, mFacingAngleDeg, worldHalfX, worldHalfY);

		float clampedX = nextX;
		float clampedY = nextY;
		constrainPlayerToTutorialOutline(
			clampedX,
			clampedY,
			worldHalfX,
			worldHalfY,
			mPlayerHitboxHalfW,
			mPlayerHitboxHalfH,
			mFacingAngleDeg);

		const float kTouchSlop = 3.0f;
		bool newContact = false;
		for (int s = 0; s < kTutorialWireSegmentCount; ++s)
		{
			const int o = s * 4;
			const float ax = kTutorialWireFlat[static_cast<std::size_t>(o + 0)];
			const float ay = kTutorialWireFlat[static_cast<std::size_t>(o + 1)];
			const float bx = kTutorialWireFlat[static_cast<std::size_t>(o + 2)];
			const float by = kTutorialWireFlat[static_cast<std::size_t>(o + 3)];
			float qx = 0.0f;
			float qy = 0.0f;
			closestPointOnSegment2D(clampedX, clampedY, ax, ay, bx, by, qx, qy);
			float mx = clampedX - qx;
			float my = clampedY - qy;
			float dist = std::sqrt(mx * mx + my * my);
			float ndx = 0.0f;
			float ndy = 0.0f;
			if (dist < 1.0e-4f)
			{
				const float abx = bx - ax;
				const float aby = by - ay;
				const float ablen = std::sqrt(abx * abx + aby * aby);
				if (ablen < 1.0e-6f)
				{
					continue;
				}
				ndx = -aby / ablen;
				ndy = abx / ablen;
			}
			else
			{
				ndx = mx / dist;
				ndy = my / dist;
			}
			const float req =
				obbExtentAlongInwardNormal(ndx, ndy, mPlayerHitboxHalfW, mPlayerHitboxHalfH, mFacingAngleDeg);
			const bool touching = dist > 0.0f && dist <= req + kTouchSlop;
			if (touching && !mOutlineWallPrevTouch[static_cast<std::size_t>(s)])
			{
				newContact = true;
			}
			mOutlineWallPrevTouch[static_cast<std::size_t>(s)] = touching;
		}
		if (newContact)
		{
			playWallHitSoundIfReady();
		}

		mPlayerX = clampedX;
		mPlayerY = clampedY;

		mPlayer->setX(static_cast<int>(mPlayerX));
		mPlayer->setY(static_cast<int>(mPlayerY));
		if (mPlayerHitboxDebug != 0)
		{
			mPlayerHitboxDebug->setX(static_cast<int>(mPlayerX));
			mPlayerHitboxDebug->setY(static_cast<int>(mPlayerY));
		}

		updateCamera();
		mRenderer->setCamera(mCameraX, mCameraY);
	}
}

void Game::draw(Renderer& renderer)
{
	++mFrameCount;

	renderer.clear();

	if (mFloor != 0)
	{
		mFloor->draw(renderer);
	}

	if (mPlayer != 0)
	{
		mPlayer->draw(renderer);
	}
	if (mShowDebugHitbox && mPlayerHitboxDebug != 0)
	{
		mPlayerHitboxDebug->draw(renderer);
	}

	renderer.drawWorldLineSegments(
		kTutorialWireFlat.data(), kTutorialWireSegmentCount, 1.0f, 0.0f, 0.0f, 1.0f);

	renderer.present();
}

namespace
{
	const float kMaxTurnRateDegPerSec = 220.0f;

	const float kSpriteFacingOffsetDeg = 90.0f;
	const float kFacingTurnSign = 1.0f;
	const float kAimSnapDegrees = 1.25f;

	float shortestAngleDeltaDegrees(float fromDeg, float toDeg)
	{
		float d = toDeg - fromDeg;
		while (d > 180.0f)
		{
			d -= 360.0f;
		}
		while (d < -180.0f)
		{
			d += 360.0f;
		}
		return d;
	}
}

void Game::updateFacingTowardMouse(float deltaTime)
{
	int mouseX = 0;
	int mouseY = 0;
	SDL_GetMouseState(&mouseX, &mouseY);

	const float worldMouseX = mCameraX + static_cast<float>(mouseX);
	const float worldMouseY = mCameraY + static_cast<float>(mouseY);

	const float dx = worldMouseX - mPlayerX;
	const float dy = worldMouseY - mPlayerY;
	const float lenSq = dx * dx + dy * dy;
	if (lenSq < 4.0f)
	{
		return;
	}

	const float targetDeg =
		-std::atan2(dy, dx) * 57.2957795f + kSpriteFacingOffsetDeg;

	const float maxStep = kMaxTurnRateDegPerSec * deltaTime;
	float delta = shortestAngleDeltaDegrees(mFacingAngleDeg, targetDeg);

	if (std::fabs(delta) <= maxStep || std::fabs(delta) <= kAimSnapDegrees)
	{
		mFacingAngleDeg = targetDeg;
	}
	else
	{
		const float step = (delta >= 0.0f) ? maxStep : -maxStep;
		mFacingAngleDeg += kFacingTurnSign * step;
	}

	while (mFacingAngleDeg >= 360.0f)
	{
		mFacingAngleDeg -= 360.0f;
	}
	while (mFacingAngleDeg < 0.0f)
	{
		mFacingAngleDeg += 360.0f;
	}
}

void Game::updateCamera()
{
	const float viewW = static_cast<float>(mRenderer->getWidth());
	const float viewH = static_cast<float>(mRenderer->getHeight());

	float maxCamX = mMapWidth - viewW;
	float maxCamY = mMapHeight - viewH;
	if (maxCamX < 0.0f) { maxCamX = 0.0f; }
	if (maxCamY < 0.0f) { maxCamY = 0.0f; }

	mCameraX = mPlayerX - viewW * 0.5f;
	mCameraY = mPlayerY - viewH * 0.5f;

	if (mCameraX < 0.0f) { mCameraX = 0.0f; }
	if (mCameraY < 0.0f) { mCameraY = 0.0f; }
	if (mCameraX > maxCamX) { mCameraX = maxCamX; }
	if (mCameraY > maxCamY) { mCameraY = maxCamY; }
}

void Game::processFrameCounting(float deltaTime)
{
	mElapsedSeconds += deltaTime;

	if (mElapsedSeconds > 1.0f)
	{
		mElapsedSeconds -= 1.0f;
		mFps = mFrameCount;
		mFrameCount = 0;
	}
}

void Game::constrainPlayerToTutorialOutline(
	float& cx,
	float& cy,
	float worldHalfX,
	float worldHalfY,
	float localHalfW,
	float localHalfH,
	float facingAngleDeg)
{
	const float maxX = mMapWidth - worldHalfX;
	const float maxY = mMapHeight - worldHalfY;
	if (cx < worldHalfX) { cx = worldHalfX; }
	if (cx > maxX) { cx = maxX; }
	if (cy < worldHalfY) { cy = worldHalfY; }
	if (cy > maxY) { cy = maxY; }

	constrainCenterToWireSegments(
		cx,
		cy,
		kTutorialWireFlat.data(),
		kTutorialWireSegmentCount,
		localHalfW,
		localHalfH,
		facingAngleDeg);
}

namespace
{
	bool audioSpecMatches(const SDL_AudioSpec& a, const SDL_AudioSpec& b)
	{
		return a.format == b.format && a.channels == b.channels && a.freq == b.freq;
	}
}

void Game::shutdownWallHitAudio()
{
	if (mWallHitAudioDevice != 0)
	{
		SDL_CloseAudioDevice(static_cast<SDL_AudioDeviceID>(mWallHitAudioDevice));
		mWallHitAudioDevice = 0;
	}
	if (mWallHitPcm != 0)
	{
		if (mWallHitPcmFromLoadWav)
		{
			SDL_FreeWAV(mWallHitPcm);
		}
		else
		{
			SDL_free(mWallHitPcm);
		}
		mWallHitPcm = 0;
		mWallHitPcmLen = 0;
		mWallHitPcmFromLoadWav = false;
	}
}

bool Game::tryInitWallHitAudio()
{
	shutdownWallHitAudio();

	SDL_AudioSpec want;
	Uint8* wavBuf = 0;
	Uint32 wavLen = 0;
	if (SDL_LoadWAV("sounds/thud.wav", &want, &wavBuf, &wavLen) == 0)
	{
		return false;
	}

	SDL_AudioSpec have;
	const Uint32 allow =
		SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_FORMAT_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE;
	SDL_AudioDeviceID dev = SDL_OpenAudioDevice(0, 0, &want, &have, allow);
	if (dev == 0)
	{
		SDL_FreeWAV(wavBuf);
		return false;
	}

	if (audioSpecMatches(want, have))
	{
		mWallHitPcm = wavBuf;
		mWallHitPcmLen = wavLen;
		mWallHitPcmFromLoadWav = true;
	}
	else
	{
		SDL_AudioCVT cvt;
		if (SDL_BuildAudioCVT(
				&cvt, want.format, want.channels, want.freq, have.format, have.channels, have.freq) < 0)
		{
			SDL_FreeWAV(wavBuf);
			SDL_CloseAudioDevice(dev);
			return false;
		}
		if (cvt.len_mult <= 0)
		{
			SDL_FreeWAV(wavBuf);
			SDL_CloseAudioDevice(dev);
			return false;
		}
		cvt.len = static_cast<int>(wavLen);
		cvt.buf = static_cast<Uint8*>(SDL_malloc(static_cast<size_t>(cvt.len) * static_cast<size_t>(cvt.len_mult)));
		if (cvt.buf == 0)
		{
			SDL_FreeWAV(wavBuf);
			SDL_CloseAudioDevice(dev);
			return false;
		}
		std::memcpy(cvt.buf, wavBuf, static_cast<size_t>(wavLen));
		SDL_FreeWAV(wavBuf);
		if (SDL_ConvertAudio(&cvt) != 0)
		{
			SDL_free(cvt.buf);
			SDL_CloseAudioDevice(dev);
			return false;
		}
		mWallHitPcm = cvt.buf;
		mWallHitPcmLen = static_cast<Uint32>(cvt.len_cvt);
		mWallHitPcmFromLoadWav = false;
	}

	mWallHitAudioDevice = static_cast<std::uint32_t>(dev);
	SDL_PauseAudioDevice(dev, 0);
	return true;
}

void Game::playWallHitSoundIfReady()
{
	if (mWallHitAudioDevice == 0 || mWallHitPcm == 0 || mWallHitPcmLen == 0)
	{
		return;
	}
	const SDL_AudioDeviceID dev = static_cast<SDL_AudioDeviceID>(mWallHitAudioDevice);
	(void)SDL_QueueAudio(dev, mWallHitPcm, mWallHitPcmLen);
}
