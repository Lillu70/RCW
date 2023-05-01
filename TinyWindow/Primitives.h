#pragma once

#include <inttypes.h>

typedef uint8_t		u8;
typedef uint16_t	u16;
typedef uint32_t	u32;
typedef uint64_t	u64;

typedef int8_t		i8;
typedef int16_t		i16;
typedef int32_t		i32;
typedef int64_t		i64;

typedef float		f32;
typedef double		f64;

typedef i32			b32;

constexpr f64 PI = 3.141592653589793;
constexpr f32 PI32 = f32(PI);
constexpr f64 TAU = 6.283185307179586;
constexpr f32 TAU32 = f32(TAU);
constexpr u32 DEG_FULL_CIRCLE = 360;
constexpr u32 DEG_HALF_CIRCLE = DEG_FULL_CIRCLE / 2;