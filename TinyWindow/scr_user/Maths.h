#pragma once
#include "Types.h"

#include <utility>
#include <cmath>
#include <limits>


inline bool line_intersection(v2f l0p0, v2f l0p1, v2f l1p0, v2f l1p1, v2f& hit_location)
{
	v2f s1;
	s1.x = l0p1.x - l0p0.x;
	s1.y = l0p1.y - l0p0.y;

	v2f s2;
	s2.x = l1p1.x - l1p0.x;
	s2.y = l1p1.y - l1p0.y;

	f32 x = -s2.x * s1.y + s1.x * s2.y;
	f32 s = (-s1.y * (l0p0.x - l1p0.x) + s1.x * (l0p0.y - l1p0.y)) / x;
	f32 t = (s2.x * (l0p0.y - l1p0.y) - s2.y * (l0p0.x - l1p0.x)) / x;

	if (s >= 0 && s <= 1 && t >= 0 && t <= 1)
	{
		hit_location.x = l0p0.x + (t * s1.x);
		hit_location.y = l0p0.y + (t * s1.y);
		return true;
	}

	return false;
}

template<typename T>
inline auto square(T val)
{
	return val * val;
}

template<typename T>
inline f32 distance(v2<T> a, v2<T> b)
{
	return std::sqrtf((f32)(square(a.x - b.x) + square(a.y - b.y)));
}

template<typename T>
inline f32 slope(v2<T> a, v2<T> b)
{					  
	return  (f32)(b.y - a.y) / (f32)(b.x - a.x);  
}

template<typename T>
inline f32 magnitude(v2<T> a)
{
	return std::sqrtf(square(a.x) + square(a.y));
}

template<typename T>
inline v2<T> normalize(v2<T> a)
{
	f32 m = magnitude(a);
	return v2<T>(a.x / m, a.y / m);
}

template<typename T>
inline v2<T> rotate_point(v2<T> p, f32 a)
{
	f32 c = cosf(a);
	f32 s = sinf(a);
	return v2<T>(p.x * c - p.y * s, p.x * s + p.y * c);
}

template<typename T>
inline v2<T> rotate_point(v2<T> p, f32 cos_a, f32 sin_a)
{
	return v2<T>(p.x * cos_a - p.y * sin_a, p.x * sin_a + p.y * cos_a);
}

template<typename T>
inline v2<T> flip_y(v2<T> p)
{
	return { p.x, p.y * -1 };
}

//Returns y, for the provided x.
template<typename T>
inline T linear_interpolation(v2<T> p1, v2<T> p2, T x)
{
	return p1.y + (T)(slope(p1, p2) * (x - p1.x));
}

inline u32 multiply_accross_color_channels(u32 color, f32 mult)
{
	u8* p = (u8*)&color;
	for (u32 i = 0; i < 3; i++)
	{
		i32 val = (i32)*(p + i);
		val = std::min(255, std::max(0, (i32)((*(p + i)) * mult)));
		*(p + i) = (u8)val;
	}

	return color;
}

inline static i32 round_to_int(f32 real)
{
	return (i32)(real + 0.5f);
}

inline static f32 rad_to_deg(f32 radian_value)
{
	return radian_value * DEG_HALF_CIRCLE / PI32;
}

inline static f32 deg_to_rad(f32 degree_value)
{
	return degree_value * PI32 / DEG_HALF_CIRCLE;
}

inline static f64 min(f64 a, f64 b) { return a < b ? a : b; }
inline static f32 min(f32 a, f32 b) { return a < b ? a : b; }
inline static i64 min(i64 a, i64 b) { return a < b ? a : b; }
inline static u64 min(u64 a, u64 b) { return a < b ? a : b; }
inline static i32 min(i32 a, i32 b) { return a < b ? a : b; }
inline static u32 min(u32 a, u32 b) { return a < b ? a : b; }
inline static i16 min(i16 a, i16 b) { return a < b ? a : b; }
inline static u16 min(u16 a, u16 b) { return a < b ? a : b; }
inline static i8  min(i8 a, i8 b)	{ return a < b ? a : b; }
inline static u8  min(u8 a, u8 b)	{ return a < b ? a : b; }

inline static f64 max(f64 a, f64 b) { return a > b ? a : b; }
inline static f32 max(f32 a, f32 b) { return a > b ? a : b; }
inline static i64 max(i64 a, i64 b) { return a > b ? a : b; }
inline static u64 max(u64 a, u64 b) { return a > b ? a : b; }
inline static i32 max(i32 a, i32 b) { return a > b ? a : b; }
inline static u32 max(u32 a, u32 b) { return a > b ? a : b; }
inline static i16 max(i16 a, i16 b) { return a > b ? a : b; }
inline static u16 max(u16 a, u16 b) { return a > b ? a : b; }
inline static i8  max(i8 a, i8 b)   { return a > b ? a : b; }
inline static u8  max(u8 a, u8 b)   { return a > b ? a : b; }