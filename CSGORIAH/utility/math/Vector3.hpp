#pragma once

struct Vector3
{
	float x, y, z;
	Vector3() : x{ 0.0f }, y{ 0.0f }, z{ 0.0 } {}
	Vector3(float _x, float _y, float _z) : x{ _x }, y{ _y }, z{ _z } {}

	static float magnitudeBeetween2Point2D(int x1, int y1, int x2, int y2)
	{
		auto x = x2 - x1;
		auto y = y2 - y1;
		return std::sqrt(x*x + y*y);
	}
};