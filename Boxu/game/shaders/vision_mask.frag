#version 330

#define MAX_WALLS 32

uniform float uSunekuX;
uniform float uSunekuY;
uniform float uFacingDeg;
uniform float uHalfConeRad;
uniform float uFeatherRad;
uniform float uOutsideAlpha;
uniform float uCamX;
uniform float uCamY;
uniform float uViewW;
uniform float uViewH;

uniform int uWallSegCount;
uniform vec4 uWallSeg[MAX_WALLS];

out vec4 outColor;

bool segmentSegmentIntersect(vec2 p1, vec2 p2, vec2 p3, vec2 p4)
{
	float denom = (p4.y - p3.y) * (p2.x - p1.x) - (p4.x - p3.x) * (p2.y - p1.y);
	if (abs(denom) < 1e-6)
	{
		return false;
	}
	float ua = ((p4.x - p3.x) * (p1.y - p3.y) - (p4.y - p3.y) * (p1.x - p3.x)) / denom;
	float ub = ((p2.x - p1.x) * (p1.y - p3.y) - (p2.y - p1.y) * (p1.x - p3.x)) / denom;
	return ua > 1e-4 && ua < 1.0 - 1e-4 && ub > 1e-4 && ub < 1.0 - 1e-4;
}

bool rayBlockedByWalls(vec2 S, vec2 P)
{
	int n = uWallSegCount;
	if (n > MAX_WALLS)
	{
		n = MAX_WALLS;
	}
	for (int i = 0; i < MAX_WALLS; ++i)
	{
		if (i >= n)
		{
			break;
		}
		vec4 seg = uWallSeg[i];
		vec2 A = seg.xy;
		vec2 B = seg.zw;
		if (segmentSegmentIntersect(S, P, A, B))
		{
			return true;
		}
	}
	return false;
}

void main()
{
	float ndcX = gl_FragCoord.x * (2.0 / uViewW) - 1.0;
	float ndcY = gl_FragCoord.y * (2.0 / uViewH) - 1.0;
	float worldX = uCamX + (ndcX + 1.0) * uViewW * 0.5;
	float worldY = uCamY + (1.0 - ndcY) * uViewH * 0.5;

	vec2 S = vec2(uSunekuX, uSunekuY);
	vec2 P = vec2(worldX, worldY);
	vec2 v = P - S;
	float len = length(v);
	if (len < 18.0)
	{
		discard;
	}

	float ang = atan(v.y, v.x);
	float lookRad = radians(90.0 - uFacingDeg);
	float d = ang - lookRad;
	d = atan(sin(d), cos(d));

	float darkAng = smoothstep(
		uHalfConeRad - uFeatherRad,
		uHalfConeRad + uFeatherRad,
		abs(d));

	float dark = darkAng;
	if (darkAng < 0.98 && rayBlockedByWalls(S, P))
	{
		dark = 1.0;
	}

	if (dark < 0.001)
	{
		discard;
	}

	outColor = vec4(0.0, 0.0, 0.0, dark * uOutsideAlpha);
}
