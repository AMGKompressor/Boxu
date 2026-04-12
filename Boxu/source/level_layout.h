// COMP710 Boxu — wall list for the red wire + collision (+Y down like the screen).
// I rebuild from Boxu/temp/build-boxu after I touch this file.

#pragma once

#include <array>
#include <cstddef>

namespace Layout
{
	// Entry gap — world X at the sides of the hole
	constexpr float kEntryWest = 80.0f;
	constexpr float kEntryEast = kEntryWest + (1000.0f - kEntryWest) * 1.35f; // I stretched east 35% past midpoint so the tunnel feels wider

	struct Wall
	{
		float x0, y0, x1, y1;
	};

	// Optional shiftX/shiftY on a row = slide the whole segment without redoing coords
	struct WallSpec
	{
		float x0, y0, x1, y1;
		float shiftX = 0.f;
		float shiftY = 0.f;
	};

	namespace detail
	{
		template <std::size_t N>
		constexpr std::array<Wall, N> buildWalls(const WallSpec (&src)[N])
		{
			std::array<Wall, N> out{};
			for (std::size_t i = 0; i < N; ++i)
			{
				const float sx = src[i].shiftX;
				const float sy = src[i].shiftY;
				out[i].x0 = src[i].x0 + sx;
				out[i].y0 = src[i].y0 + sy;
				out[i].x1 = src[i].x1 + sx;
				out[i].y1 = src[i].y1 + sy;
			}
			return out;
		}
	}

	// Lazy helpers for flat / plumb walls — diagonals stay as raw { x0,y0,x1,y1 }
	constexpr WallSpec Hor(float y, float xa, float xb) { return { xa, y, xb, y }; }
	constexpr WallSpec Ver(float x, float ya, float yb) { return { x, ya, x, yb }; }

	inline constexpr WallSpec kWallData[] = {
		Hor(1200.0f, kEntryWest, kEntryEast),     // entry floor
		Ver(kEntryEast, 1200.0f, 1500.0f),        // east edge straight down
		Hor(1500.0f, 1320.0f, 2480.0f),          // main room bottom
		Ver(2480.0f, 1500.0f, 190.0f),            // right outer wall
		Hor(190.0f, 2480.0f, 1750.0f),            // top strip
		{ 1750.0f, 190.0f, kEntryEast, 720.0f },   // upper diagonal
		Hor(720.0f, kEntryEast, kEntryWest),      // entry ceiling
		Ver(kEntryWest, 720.0f, 880.0f),         // left wall above the gap
		Ver(kEntryWest, 1040.0f, 1200.0f),       // left wall under the gap
		Hor(960.0f, kEntryEast, 2480.0f),         // horizontal divider
	};

	inline constexpr auto kWalls = detail::buildWalls(kWallData);
	constexpr int kWallCount = static_cast<int>(kWalls.size());
}
