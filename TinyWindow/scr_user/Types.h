#pragma once

#include "../Types.h"

constexpr f64 PI = 3.141592653589793;
constexpr f64 TAU = 6.283185307179586;

#define WHITE 0xffffffff
#define BLACK 0xff000000
#define RED   0xffff0000
#define GREEN 0xff00ff00
#define BLUE  0xff0000ff

template<typename T>
struct v2
{
	v2() = default;
	v2(T x, T y) : x(x), y(y) {}
	v2(const v2<T>& other) : x(other.x), y(other.y) {}

	T x = 0;
	T y = 0;

	template<typename U>
	inline v2<U> As()
	{
		return v2<U>((U)x, (U)y);
	}

	v2<T> operator * (const v2<T>& other)	const	{ return { x * other.x, y * other.y };	}
	v2<T> operator * (const T& comp)		const	{ return { x * comp,	y * comp	};	}
	v2<T> operator / (const v2<T>& other)	const	{ return { x / other.x, y / other.y };	}
	v2<T> operator / (const T& comp)		const	{ return { x / comp,	y / comp	};	}
	v2<T> operator + (const v2<T>& other)	const	{ return { x + other.x, y + other.y };	}
	v2<T> operator + (const T& comp)		const	{ return { x + comp,	y + comp	};	}
	v2<T> operator - (const v2<T>& other)	const	{ return { x - other.x, y - other.y };	}
	v2<T> operator - (const T& comp)		const	{ return { x - comp,	y - comp	};	}
};

typedef v2<i32> v2i;
typedef v2<u32> v2u;
typedef v2<f32> v2f;

