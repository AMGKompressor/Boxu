// COMP710 GP Framework — field op codename Suneku

#include "game.h"

#include <SDL.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <vector>

#include "renderer.h"
#include "texture.h"
#include "logmanager.h"
#include "sprite.h"

namespace
{
	const float kWalkMoveSpeed = 130.0f;
	const float kSprintMoveSpeedMax = 300.0f;
	const float kMoveSpeedRampUpPerSec = 260.0f;
	const float kMoveSpeedRampDownPerSec = 320.0f;

	const float kWallNoiseRadiusWalk = 140.0f;
	const float kWallNoiseRadiusSprint = 320.0f;
	const float kNoisePulseDuration = 0.55f;

	const float kOcelotSpawnX = 2000.0f;
	const float kOcelotSpawnY = 480.0f;
	const float kOcelotBodyRadius = 38.0f;
	const float kOcelotChaseSpeed = 108.0f;
	const float kOcelotStopDist = 72.0f;

	const float kVisionConeHalfAngleDeg = 40.0f;
	const float kVisionConeFeatherDeg = 3.5f;
	const float kVisionOutsideAlpha = 0.94f;

	const float kSunekuBodyBrownR = 0.24f;
	const float kSunekuBodyBrownG = 0.14f;
	const float kSunekuBodyBrownB = 0.085f;

	const float kMapFloorGreyR = 0.34f;
	const float kMapFloorGreyG = 0.34f;
	const float kMapFloorGreyB = 0.38f;

	constexpr float kKeyCardWorldX = 1880.0f;
	constexpr float kKeyCardWorldY = 540.0f;
	constexpr float kKeyCardPickupRadius = 52.0f;
	constexpr float kMissionExtractCenterX = 2000.0f;
	constexpr float kMissionExtractCenterY = 1120.0f;
	constexpr float kMissionExtractHalfW = 130.0f;
	constexpr float kMissionExtractHalfH = 88.0f;
	constexpr float kMissionCatchDistance = 66.0f;

	void drawWorldCircleOutline(
		Renderer& renderer,
		float cx,
		float cy,
		float radius,
		int segments,
		float cr,
		float cg,
		float cb,
		float ca)
	{
		if (segments < 3 || segments > 64)
		{
			return;
		}
		float xy[128];
		const float twoPi = 6.2831853f;
		for (int i = 0; i < segments; ++i)
		{
			const float ang = twoPi * static_cast<float>(i) / static_cast<float>(segments);
			xy[i * 2 + 0] = cx + std::cos(ang) * radius;
			xy[i * 2 + 1] = cy + std::sin(ang) * radius;
		}
		renderer.drawWorldLineLoop(xy, segments, cr, cg, cb, ca);
	}

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
	static_assert(kTutorialWireSegmentCount == Game::kTutorialWallPhysicsSegmentCount, 		"layout vs game.h wall count");

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

	bool glyphRows5x7(char c, std::array<std::uint8_t, 7>& rows)
	{
		switch (c)
		{
		case '0':
			rows = {14, 17, 17, 17, 17, 17, 14};
			return true;
		case '1':
			rows = {4, 12, 4, 4, 4, 4, 14};
			return true;
		case '.':
			rows = {0, 0, 0, 0, 0, 12, 12};
			return true;
		case ' ':
			rows.fill(0);
			return true;
		case 'a':
			rows = {14, 1, 15, 17, 17, 15, 0};
			return true;
		case 'l':
			rows = {4, 4, 4, 4, 4, 4, 7};
			return true;
		case 'p':
			rows = {14, 17, 17, 17, 14, 16, 16};
			return true;
		case 'h':
			rows = {16, 16, 16, 31, 17, 17, 17};
			return true;
		case 'A':
			rows = {14, 17, 17, 31, 17, 17, 17};
			return true;
		case 'B':
			rows = {30, 17, 17, 30, 17, 17, 30};
			return true;
		case 'C':
			rows = {14, 17, 16, 16, 16, 17, 14};
			return true;
		case 'D':
			rows = {30, 17, 17, 17, 17, 17, 30};
			return true;
		case 'E':
			rows = {31, 16, 28, 16, 16, 16, 31};
			return true;
		case 'F':
			rows = {31, 16, 28, 16, 16, 16, 16};
			return true;
		case 'G':
			rows = {14, 17, 16, 23, 17, 17, 14};
			return true;
		case 'H':
			rows = {17, 17, 17, 31, 17, 17, 17};
			return true;
		case 'I':
			rows = {14, 4, 4, 4, 4, 4, 14};
			return true;
		case 'J':
			rows = {30, 1, 1, 1, 17, 17, 14};
			return true;
		case 'K':
			rows = {17, 18, 20, 24, 20, 18, 17};
			return true;
		case 'L':
			rows = {16, 16, 16, 16, 16, 16, 31};
			return true;
		case 'M':
			rows = {17, 27, 21, 17, 17, 17, 17};
			return true;
		case 'N':
			rows = {17, 19, 21, 25, 17, 17, 17};
			return true;
		case 'O':
			rows = {14, 17, 17, 17, 17, 17, 14};
			return true;
		case 'P':
			rows = {30, 17, 17, 30, 16, 16, 16};
			return true;
		case 'Q':
			rows = {14, 17, 17, 21, 19, 17, 14};
			return true;
		case 'R':
			rows = {30, 17, 17, 30, 20, 18, 17};
			return true;
		case 'S':
			rows = {14, 17, 16, 14, 1, 17, 14};
			return true;
		case 'T':
			rows = {31, 4, 4, 4, 4, 4, 4};
			return true;
		case 'U':
			rows = {17, 17, 17, 17, 17, 17, 14};
			return true;
		case 'V':
			rows = {17, 17, 17, 17, 10, 10, 4};
			return true;
		case 'W':
			rows = {17, 17, 17, 17, 21, 27, 17};
			return true;
		case 'X':
			rows = {17, 17, 10, 4, 10, 17, 17};
			return true;
		case 'Y':
			rows = {17, 17, 10, 4, 4, 4, 4};
			return true;
		case 'Z':
			rows = {31, 1, 2, 4, 8, 16, 31};
			return true;
		case ':':
			rows = {0, 12, 12, 0, 12, 12, 0};
			return true;
		case '=':
			rows = {0, 0, 31, 0, 31, 0, 0};
			return true;
		default:
			return false;
		}
	}

	void setRgbaPixel(std::vector<unsigned char>& buf, int w, int h, int x, int y, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
	{
		if (x < 0 || y < 0 || x >= w || y >= h || a == 0)
		{
			return;
		}
		const int i = (y * w + x) * 4;
		const std::size_t u = static_cast<std::size_t>(i);
		if (a == 255 || buf[u + 3] == 0)
		{
			buf[u + 0] = r;
			buf[u + 1] = g;
			buf[u + 2] = b;
			buf[u + 3] = a;
			return;
		}
		const float fa = static_cast<float>(a) / 255.0f;
		const float inv = 1.0f - fa;
		const float br = static_cast<float>(buf[u + 0]);
		const float bg = static_cast<float>(buf[u + 1]);
		const float bb = static_cast<float>(buf[u + 2]);
		const float ba = static_cast<float>(buf[u + 3]);
		buf[u + 0] = static_cast<unsigned char>(static_cast<float>(r) * fa + br * inv);
		buf[u + 1] = static_cast<unsigned char>(static_cast<float>(g) * fa + bg * inv);
		buf[u + 2] = static_cast<unsigned char>(static_cast<float>(b) * fa + bb * inv);
		buf[u + 3] = static_cast<unsigned char>(static_cast<float>(a) * fa + ba * inv);
	}

	void flipImageRowsY(std::vector<unsigned char>& rgba, int outW, int outH)
	{
		for (int y = 0; y < outH / 2; ++y)
		{
			const int y2 = outH - 1 - y;
			for (int x = 0; x < outW; ++x)
			{
				for (int k = 0; k < 4; ++k)
				{
					const std::size_t a = static_cast<std::size_t>((y * outW + x) * 4 + k);
					const std::size_t b = static_cast<std::size_t>((y2 * outW + x) * 4 + k);
					std::swap(rgba[a], rgba[b]);
				}
			}
		}
	}

	bool buildLabelRgbaMultiline(
		const char* text,
		int scale,
		int pad,
		int charGap,
		int lineGap,
		std::vector<unsigned char>& rgba,
		int& outW,
		int& outH)
	{
		if (text == nullptr || text[0] == '\0' || scale < 1)
		{
			return false;
		}
		const int rowH = 7 * scale;
		const int cellW = 5 * scale + charGap;
		int lineCount = 1;
		for (const char* t = text; *t != '\0'; ++t)
		{
			if (*t == '\n')
			{
				++lineCount;
			}
		}
		int maxPixW = 0;
		const char* scan = text;
		while (*scan != '\0')
		{
			const char* lineStart = scan;
			while (*scan != '\0' && *scan != '\n')
			{
				++scan;
			}
			const int len = static_cast<int>(scan - lineStart);
			const int lineW = (len <= 0) ? (pad * 2) : (pad * 2 + len * cellW - charGap);
			if (lineW > maxPixW)
			{
				maxPixW = lineW;
			}
			if (*scan == '\n')
			{
				++scan;
			}
		}
		outW = maxPixW;
		outH = pad * 2 + lineCount * rowH + (lineCount - 1) * lineGap;
		rgba.assign(static_cast<std::size_t>(outW * outH * 4), 0);

		const char* p = text;
		int penY = pad;
		while (*p != '\0')
		{
			const char* lineStart = p;
			while (*p != '\0' && *p != '\n')
			{
				++p;
			}
			const int len = static_cast<int>(p - lineStart);
			const int linePixelW = (len <= 0) ? (pad * 2) : (pad * 2 + len * cellW - charGap);
			int penX = pad + (outW - linePixelW) / 2;
			for (int ci = 0; ci < len; ++ci)
			{
				const char ch = lineStart[ci];
				std::array<std::uint8_t, 7> gr{};
				if (!glyphRows5x7(ch, gr))
				{
					gr.fill(0);
				}
				for (int py = 0; py < 7; ++py)
				{
					for (int px = 0; px < 5; ++px)
					{
						const int bit = (gr[static_cast<std::size_t>(py)] >> (4 - px)) & 1;
						if (bit == 0)
						{
							continue;
						}
						for (int sy = 0; sy < scale; ++sy)
						{
							for (int sx = 0; sx < scale; ++sx)
							{
								const int x0 = penX + px * scale + sx;
								const int y0 = penY + py * scale + sy;
								setRgbaPixel(rgba, outW, outH, x0 + 1, y0 + 1, 0, 0, 0, 200);
								setRgbaPixel(rgba, outW, outH, x0, y0, 236, 236, 236, 252);
							}
						}
					}
				}
				penX += cellW;
			}
			if (*p == '\n')
			{
				++p;
			}
			penY += rowH + lineGap;
		}

		flipImageRowsY(rgba, outW, outH);
		return true;
	}

	bool tryCreateLabelSprite(Texture*& tex, Sprite*& spr, const char* text, int scale, int lineGap)
	{
		std::vector<unsigned char> buf;
		int w = 0;
		int h = 0;
		if (!buildLabelRgbaMultiline(text, scale, 4, 3, lineGap, buf, w, h))
		{
			return false;
		}
		tex = new Texture();
		if (!tex->initializeFromRgba(w, h, buf.data()))
		{
			delete tex;
			tex = 0;
			return false;
		}
		spr = new Sprite();
		if (!spr->initialize(*tex))
		{
			delete spr;
			spr = 0;
			delete tex;
			tex = 0;
			return false;
		}
		spr->setAngle(0.0f);
		spr->setScale(1.0f);
		spr->setRedTint(1.0f);
		spr->setGreenTint(1.0f);
		spr->setBlueTint(1.0f);
		spr->setAlpha(1.0f);
		return true;
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
	, mSuneku(0)
	, mSunekuHitboxDebug(0)
	, mVersionTexture(0)
	, mVersionSprite(0)
	, mHudObjectiveTexture(0)
	, mHudObjectiveSprite(0)
	, mHudWinTexture(0)
	, mHudWinSprite(0)
	, mHudLoseTexture(0)
	, mHudLoseSprite(0)
	, mSunekuX(0.0f)
	, mSunekuY(0.0f)
	, mSunekuHitboxHalfW(48.0f)
	, mSunekuHitboxHalfH(48.0f)
	, mMapWidth(2560.0f)
	, mMapHeight(1920.0f)
	, mCameraX(0.0f)
	, mCameraY(0.0f)
	, mSunekuFacingDeg(0.0f)
	, mSunekuMoveSpeed(kWalkMoveSpeed)
	, mShowSunekuHitbox(true)
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
	, mLastWallHitNoiseRadius(0.0f)
	, mWallNoisePulseAge(0.0f)
	, mWallNoisePulseMaxR(0.0f)
	, mWallNoisePulseCx(0.0f)
	, mWallNoisePulseCy(0.0f)
	, mWallNoisePulseActive(false)
	, mOcelotX(kOcelotSpawnX)
	, mOcelotY(kOcelotSpawnY)
	, mOcelotAwake(false)
	, mMissionState(MissionState::Playing)
	, mHasKeyCard(false)
	, mKeyCardX(kKeyCardWorldX)
	, mKeyCardY(kKeyCardWorldY)
	, mExtractCenterX(kMissionExtractCenterX)
	, mExtractCenterY(kMissionExtractCenterY)
	, mExtractHalfW(kMissionExtractHalfW)
	, mExtractHalfH(kMissionExtractHalfH)
{
}

Game::~Game()
{
	shutdownWallHitAudio();

	delete mSunekuHitboxDebug;
	mSunekuHitboxDebug = 0;

	delete mSuneku;
	mSuneku = 0;

	delete mFloor;
	mFloor = 0;

	delete mVersionSprite;
	mVersionSprite = 0;
	delete mVersionTexture;
	mVersionTexture = 0;

	delete mHudObjectiveSprite;
	mHudObjectiveSprite = 0;
	delete mHudObjectiveTexture;
	mHudObjectiveTexture = 0;
	delete mHudWinSprite;
	mHudWinSprite = 0;
	delete mHudWinTexture;
	mHudWinTexture = 0;
	delete mHudLoseSprite;
	mHudLoseSprite = 0;
	delete mHudLoseTexture;
	mHudLoseTexture = 0;

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

	mSuneku = mRenderer->createSprite("textures/board8x8.png");
	if (mSuneku != 0)
	{
		const float desiredSunekuBoxSize = 96.0f;
		const float sunekuScale = desiredSunekuBoxSize / static_cast<float>(mSuneku->getWidth());
		mSuneku->setScale(sunekuScale);
		mSuneku->setRedTint(kSunekuBodyBrownR);
		mSuneku->setGreenTint(kSunekuBodyBrownG);
		mSuneku->setBlueTint(kSunekuBodyBrownB);

		mSunekuHitboxHalfW = static_cast<float>(mSuneku->getWidth()) * 0.5f;
		mSunekuHitboxHalfH = static_cast<float>(mSuneku->getHeight()) * 0.5f;

		mSunekuHitboxDebug = mRenderer->createSprite("textures/board8x8.png");
		if (mSunekuHitboxDebug != 0)
		{
			mSunekuHitboxDebug->setScale(sunekuScale);
			mSunekuHitboxDebug->setRedTint(1.0f);
			mSunekuHitboxDebug->setGreenTint(0.0f);
			mSunekuHitboxDebug->setBlueTint(0.0f);
			mSunekuHitboxDebug->setAlpha(0.3f);
		}

		mSunekuX = (Game::kTutorialEntryWestX + Game::kTutorialEntryEastX) * 0.5f;
		mSunekuY = 900.0f;
		mSuneku->setX(static_cast<int>(mSunekuX));
		mSuneku->setY(static_cast<int>(mSunekuY));
		if (mSunekuHitboxDebug != 0)
		{
			mSunekuHitboxDebug->setX(static_cast<int>(mSunekuX));
			mSunekuHitboxDebug->setY(static_cast<int>(mSunekuY));
		}
		updateCamera();
		mRenderer->setCamera(mCameraX, mCameraY);
	}
	else
	{
		LogManager::getInstance().log("Failed to create Suneku sprite.");
	}

	if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
	{
		LogManager::getInstance().log("SDL audio unavailable (continuing without wall-hit sound).");
	}
	else if (!tryInitWallHitAudio())
	{
		LogManager::getInstance().log("Wall-hit sound not loaded (sounds/thud.wav).");
	}

	mVersionTexture = new Texture();
	mVersionSprite = new Sprite();
	std::vector<unsigned char> versionRgba;
	int vrW = 0;
	int vrH = 0;
	const char* kVersionOverlayText = "0.1 alpha";
	if (!buildLabelRgbaMultiline(kVersionOverlayText, 3, 4, 3, 0, versionRgba, vrW, vrH))
	{
		LogManager::getInstance().log("Version label raster failed.");
		delete mVersionSprite;
		mVersionSprite = 0;
		delete mVersionTexture;
		mVersionTexture = 0;
	}
	else if (!mVersionTexture->initializeFromRgba(vrW, vrH, versionRgba.data()))
	{
		LogManager::getInstance().log("Version label texture upload failed.");
		delete mVersionSprite;
		mVersionSprite = 0;
		delete mVersionTexture;
		mVersionTexture = 0;
	}
	else if (!mVersionSprite->initialize(*mVersionTexture))
	{
		LogManager::getInstance().log("Version label sprite init failed.");
		delete mVersionSprite;
		mVersionSprite = 0;
		delete mVersionTexture;
		mVersionTexture = 0;
	}
	else
	{
		mVersionSprite->setAngle(0.0f);
		mVersionSprite->setScale(1.0f);
		mVersionSprite->setRedTint(1.0f);
		mVersionSprite->setGreenTint(1.0f);
		mVersionSprite->setBlueTint(1.0f);
		mVersionSprite->setAlpha(0.95f);
	}

	if (!tryCreateLabelSprite(
			mHudObjectiveTexture,
			mHudObjectiveSprite,
			"GET KEY CARD\nGO SOUTH TO EXIT\nR: RESTART",
			2,
			4))
	{
		LogManager::getInstance().log("HUD objective label failed to build.");
	}
	if (!tryCreateLabelSprite(mHudWinTexture, mHudWinSprite, "MISSION\nCOMPLETE\nPRESS R", 4, 6))
	{
		LogManager::getInstance().log("HUD win label failed to build.");
	}
	if (!tryCreateLabelSprite(mHudLoseTexture, mHudLoseSprite, "CAUGHT\nPRESS R", 4, 6))
	{
		LogManager::getInstance().log("HUD lose label failed to build.");
	}

	resetMission();
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
				mShowSunekuHitbox = !mShowSunekuHitbox;
			}
			else if (event.key.keysym.scancode == SDL_SCANCODE_R)
			{
				resetMission();
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

void Game::resetMission()
{
	mMissionState = MissionState::Playing;
	mHasKeyCard = false;
	mOcelotX = kOcelotSpawnX;
	mOcelotY = kOcelotSpawnY;
	mOcelotAwake = false;
	mKeyCardX = kKeyCardWorldX;
	mKeyCardY = kKeyCardWorldY;
	mExtractCenterX = kMissionExtractCenterX;
	mExtractCenterY = kMissionExtractCenterY;
	mExtractHalfW = kMissionExtractHalfW;
	mExtractHalfH = kMissionExtractHalfH;
	mWallNoisePulseActive = false;
	mWallNoisePulseAge = 0.0f;
	mLastWallHitNoiseRadius = 0.0f;
	mOutlineWallPrevTouch.fill(false);

	if (mSuneku != 0)
	{
		mSunekuX = (Game::kTutorialEntryWestX + Game::kTutorialEntryEastX) * 0.5f;
		mSunekuY = 900.0f;
		mSunekuMoveSpeed = kWalkMoveSpeed;
		mSuneku->setX(static_cast<int>(mSunekuX));
		mSuneku->setY(static_cast<int>(mSunekuY));
		if (mSunekuHitboxDebug != 0)
		{
			mSunekuHitboxDebug->setX(static_cast<int>(mSunekuX));
			mSunekuHitboxDebug->setY(static_cast<int>(mSunekuY));
		}
	}
	if (mRenderer != 0)
	{
		updateCamera();
		mRenderer->setCamera(mCameraX, mCameraY);
	}
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

	if (mSuneku != 0)
	{
		if (mMissionState != MissionState::Playing)
		{
			updateCamera();
			mRenderer->setCamera(mCameraX, mCameraY);
			return;
		}
		SDL_PumpEvents();
		updateSunekuFacingTowardMouse(deltaTime);
		mSuneku->setAngle(mSunekuFacingDeg);
		if (mSunekuHitboxDebug != 0)
		{
			mSunekuHitboxDebug->setAngle(mSunekuFacingDeg);
		}

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

		const bool moveInput = (dx != 0.0f || dy != 0.0f);
		const bool sprintHeld =
			keys[SDL_SCANCODE_LSHIFT] != 0u || keys[SDL_SCANCODE_RSHIFT] != 0u;

		const float targetSpeed =
			(moveInput && sprintHeld) ? kSprintMoveSpeedMax : kWalkMoveSpeed;

		if (mSunekuMoveSpeed < targetSpeed)
		{
			mSunekuMoveSpeed = std::min(targetSpeed, mSunekuMoveSpeed + kMoveSpeedRampUpPerSec * deltaTime);
		}
		else if (mSunekuMoveSpeed > targetSpeed)
		{
			mSunekuMoveSpeed = std::max(targetSpeed, mSunekuMoveSpeed - kMoveSpeedRampDownPerSec * deltaTime);
		}

		const float nextX = mSunekuX + dx * mSunekuMoveSpeed * deltaTime;
		const float nextY = mSunekuY + dy * mSunekuMoveSpeed * deltaTime;

		float worldHalfX = 0.0f;
		float worldHalfY = 0.0f;
		orientedRectWorldAabbHalfExtents(
			mSunekuHitboxHalfW, mSunekuHitboxHalfH, mSunekuFacingDeg, worldHalfX, worldHalfY);

		float clampedX = nextX;
		float clampedY = nextY;
		constrainSunekuToTutorialOutline(
			clampedX,
			clampedY,
			worldHalfX,
			worldHalfY,
			mSunekuHitboxHalfW,
			mSunekuHitboxHalfH,
			mSunekuFacingDeg);

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
				obbExtentAlongInwardNormal(ndx, ndy, mSunekuHitboxHalfW, mSunekuHitboxHalfH, mSunekuFacingDeg);
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
			mLastWallHitNoiseRadius =
				sprintHeld ? kWallNoiseRadiusSprint : kWallNoiseRadiusWalk;
			mWallNoisePulseActive = true;
			mWallNoisePulseAge = 0.0f;
			mWallNoisePulseMaxR = mLastWallHitNoiseRadius;
			mWallNoisePulseCx = clampedX;
			mWallNoisePulseCy = clampedY;

			if (!mOcelotAwake)
			{
				const float edx = mOcelotX - clampedX;
				const float edy = mOcelotY - clampedY;
				const float noiseR = mLastWallHitNoiseRadius;
				if (edx * edx + edy * edy <= noiseR * noiseR)
				{
					mOcelotAwake = true;
				}
			}
		}

		if (mWallNoisePulseActive)
		{
			mWallNoisePulseAge += deltaTime;
			if (mWallNoisePulseAge >= kNoisePulseDuration)
			{
				mWallNoisePulseActive = false;
			}
		}

		mSunekuX = clampedX;
		mSunekuY = clampedY;

		mSuneku->setX(static_cast<int>(mSunekuX));
		mSuneku->setY(static_cast<int>(mSunekuY));
		if (mSunekuHitboxDebug != 0)
		{
			mSunekuHitboxDebug->setX(static_cast<int>(mSunekuX));
			mSunekuHitboxDebug->setY(static_cast<int>(mSunekuY));
		}

		if (mOcelotAwake)
		{
			float edx = mSunekuX - mOcelotX;
			float edy = mSunekuY - mOcelotY;
			const float elenSq = edx * edx + edy * edy;
			if (elenSq > kOcelotStopDist * kOcelotStopDist)
			{
				const float elen = std::sqrt(elenSq);
				edx /= elen;
				edy /= elen;
				mOcelotX += edx * kOcelotChaseSpeed * deltaTime;
				mOcelotY += edy * kOcelotChaseSpeed * deltaTime;
			}
			const float er = kOcelotBodyRadius;
			if (mOcelotX < er) { mOcelotX = er; }
			if (mOcelotY < er) { mOcelotY = er; }
			if (mOcelotX > mMapWidth - er) { mOcelotX = mMapWidth - er; }
			if (mOcelotY > mMapHeight - er) { mOcelotY = mMapHeight - er; }
		}

		if (mMissionState == MissionState::Playing)
		{
			if (!mHasKeyCard)
			{
				const float kdx = mSunekuX - mKeyCardX;
				const float kdy = mSunekuY - mKeyCardY;
				if (kdx * kdx + kdy * kdy <= kKeyCardPickupRadius * kKeyCardPickupRadius)
				{
					mHasKeyCard = true;
				}
			}

			if (mOcelotAwake)
			{
				const float odx = mSunekuX - mOcelotX;
				const float ody = mSunekuY - mOcelotY;
				if (odx * odx + ody * ody <= kMissionCatchDistance * kMissionCatchDistance)
				{
					mMissionState = MissionState::Lost;
				}
			}

			if (mMissionState == MissionState::Playing && mHasKeyCard)
			{
				const float ax = std::fabs(mSunekuX - mExtractCenterX);
				const float ay = std::fabs(mSunekuY - mExtractCenterY);
				if (ax <= mExtractHalfW && ay <= mExtractHalfH)
				{
					mMissionState = MissionState::Won;
				}
			}
		}

		updateCamera();
		mRenderer->setCamera(mCameraX, mCameraY);
	}
}

void Game::draw(Renderer& renderer)
{
	++mFrameCount;

	renderer.clear();

	renderer.drawWorldAxisAlignedQuad(
		mMapWidth * 0.5f,
		mMapHeight * 0.5f,
		mMapWidth * 0.5f,
		mMapHeight * 0.5f,
		kMapFloorGreyR,
		kMapFloorGreyG,
		kMapFloorGreyB,
		1.0f);

	if (mFloor != 0)
	{
		mFloor->draw(renderer);
	}

	renderer.drawWorldLineSegments(
		kTutorialWireFlat.data(), kTutorialWireSegmentCount, 1.0f, 0.0f, 0.0f, 1.0f);

	{
		const float exA = mHasKeyCard ? 0.22f : 0.12f;
		renderer.drawWorldAxisAlignedQuad(
			mExtractCenterX,
			mExtractCenterY,
			mExtractHalfW,
			mExtractHalfH,
			0.15f,
			0.75f,
			0.35f,
			exA);
		if (mHasKeyCard)
		{
			const float x0 = mExtractCenterX - mExtractHalfW;
			const float y0 = mExtractCenterY - mExtractHalfH;
			const float x1 = mExtractCenterX + mExtractHalfW;
			const float y1 = mExtractCenterY + mExtractHalfH;
			const float rect[] = {
				x0, y0, x1, y0,
				x1, y0, x1, y1,
				x1, y1, x0, y1,
				x0, y1, x0, y0
			};
			renderer.drawWorldLineSegments(rect, 4, 0.3f, 1.0f, 0.45f, 0.95f);
		}
	}

	if (!mHasKeyCard)
	{
		renderer.drawWorldAxisAlignedQuad(
			mKeyCardX,
			mKeyCardY,
			22.0f,
			14.0f,
			0.25f,
			0.55f,
			0.95f,
			0.92f);
		renderer.drawWorldAxisAlignedQuad(
			mKeyCardX + 6.0f,
			mKeyCardY - 2.0f,
			10.0f,
			6.0f,
			0.95f,
			0.95f,
			0.95f,
			0.5f);
	}

	{
		const float er = kOcelotBodyRadius;
		if (mOcelotAwake)
		{
			drawWorldCircleOutline(renderer, mOcelotX, mOcelotY, er, 36, 1.0f, 0.32f, 0.08f, 1.0f);
			drawWorldCircleOutline(renderer, mOcelotX, mOcelotY, er * 0.55f, 24, 1.0f, 0.55f, 0.2f, 0.85f);
		}
		else
		{
			drawWorldCircleOutline(renderer, mOcelotX, mOcelotY, er, 36, 0.42f, 0.45f, 0.5f, 0.75f);
		}
	}

	if (!mShowSunekuHitbox)
	{
		renderer.drawVisionConeMask(
			mSunekuX,
			mSunekuY,
			mSunekuFacingDeg,
			kVisionConeHalfAngleDeg,
			kVisionConeFeatherDeg,
			kVisionOutsideAlpha,
			mCameraX,
			mCameraY,
			kTutorialWireFlat.data(),
			kTutorialWireSegmentCount);
	}

	if (mSuneku != 0)
	{
		mSuneku->draw(renderer);
	}
	if (mShowSunekuHitbox && mSunekuHitboxDebug != 0)
	{
		mSunekuHitboxDebug->draw(renderer);
	}

	if (mShowSunekuHitbox && mWallNoisePulseActive && mWallNoisePulseMaxR > 1.0f)
	{
		const float t = mWallNoisePulseAge / kNoisePulseDuration;
		if (t >= 0.0f && t < 1.0f)
		{
			const float ringR = mWallNoisePulseMaxR * t;
			const float alpha = (1.0f - t) * 0.9f;
			const int n = 36;
			float xy[72];
			const float twoPi = 6.2831853f;
			for (int i = 0; i < n; ++i)
			{
				const float ang = twoPi * static_cast<float>(i) / static_cast<float>(n);
				xy[i * 2 + 0] = mWallNoisePulseCx + std::cos(ang) * ringR;
				xy[i * 2 + 1] = mWallNoisePulseCy + std::sin(ang) * ringR;
			}
			renderer.drawWorldLineLoop(xy, n, 0.35f, 0.88f, 1.0f, alpha);
		}
	}

	const float viewW = static_cast<float>(renderer.getWidth());
	const float viewH = static_cast<float>(renderer.getHeight());
	const float margin = 14.0f;

	if (mHudObjectiveSprite != 0 && mMissionState == MissionState::Playing)
	{
		const int ow = mHudObjectiveSprite->getWidth();
		const int oh = mHudObjectiveSprite->getHeight();
		const float ocx = mCameraX + margin + static_cast<float>(ow) * 0.5f;
		const float ocy = mCameraY + viewH - margin - static_cast<float>(oh) * 0.5f;
		mHudObjectiveSprite->setX(static_cast<int>(ocx));
		mHudObjectiveSprite->setY(static_cast<int>(ocy));
		mHudObjectiveSprite->setAlpha(0.92f);
		mHudObjectiveSprite->draw(renderer);
	}

	if (mMissionState == MissionState::Won && mHudWinSprite != 0)
	{
		const float wcx = mCameraX + viewW * 0.5f;
		const float wcy = mCameraY + viewH * 0.5f;
		mHudWinSprite->setX(static_cast<int>(wcx));
		mHudWinSprite->setY(static_cast<int>(wcy));
		mHudWinSprite->setAlpha(1.0f);
		mHudWinSprite->draw(renderer);
	}
	else if (mMissionState == MissionState::Lost && mHudLoseSprite != 0)
	{
		const float lcx = mCameraX + viewW * 0.5f;
		const float lcy = mCameraY + viewH * 0.5f;
		mHudLoseSprite->setX(static_cast<int>(lcx));
		mHudLoseSprite->setY(static_cast<int>(lcy));
		mHudLoseSprite->setAlpha(1.0f);
		mHudLoseSprite->draw(renderer);
	}

	if (mVersionSprite != 0)
	{
		const int sw = mVersionSprite->getWidth();
		const int sh = mVersionSprite->getHeight();
		const float cx = mCameraX + viewW - margin - static_cast<float>(sw) * 0.5f;
		const float cy = mCameraY + margin + static_cast<float>(sh) * 0.5f;
		mVersionSprite->setX(static_cast<int>(cx));
		mVersionSprite->setY(static_cast<int>(cy));
		mVersionSprite->draw(renderer);
	}

	renderer.present();
}

namespace
{
	const float kMaxTurnRateDegPerSec = 220.0f;

	const float kSunekuSpriteFacingOffsetDeg = 90.0f;
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

void Game::updateSunekuFacingTowardMouse(float deltaTime)
{
	int mouseX = 0;
	int mouseY = 0;
	SDL_GetMouseState(&mouseX, &mouseY);

	const float worldMouseX = mCameraX + static_cast<float>(mouseX);
	const float worldMouseY = mCameraY + static_cast<float>(mouseY);

	const float dx = worldMouseX - mSunekuX;
	const float dy = worldMouseY - mSunekuY;
	const float lenSq = dx * dx + dy * dy;
	if (lenSq < 4.0f)
	{
		return;
	}

	const float targetDeg =
		-std::atan2(dy, dx) * 57.2957795f + kSunekuSpriteFacingOffsetDeg;

	const float maxStep = kMaxTurnRateDegPerSec * deltaTime;
	float delta = shortestAngleDeltaDegrees(mSunekuFacingDeg, targetDeg);

	if (std::fabs(delta) <= maxStep || std::fabs(delta) <= kAimSnapDegrees)
	{
		mSunekuFacingDeg = targetDeg;
	}
	else
	{
		const float step = (delta >= 0.0f) ? maxStep : -maxStep;
		mSunekuFacingDeg += kFacingTurnSign * step;
	}

	while (mSunekuFacingDeg >= 360.0f)
	{
		mSunekuFacingDeg -= 360.0f;
	}
	while (mSunekuFacingDeg < 0.0f)
	{
		mSunekuFacingDeg += 360.0f;
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

	mCameraX = mSunekuX - viewW * 0.5f;
	mCameraY = mSunekuY - viewH * 0.5f;

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

void Game::constrainSunekuToTutorialOutline(
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
