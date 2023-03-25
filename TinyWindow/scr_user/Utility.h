#pragma once
#include <iostream>

template<typename T>
inline void Print_As_Binary(T val)
{
	for (u32 i = sizeof(val) - 1; i < sizeof(val); i--)
	{
		for (u8 b = 7; b < 8; b--)
			std::cout << u32((*((u8*)&val + i) & (1 << b)) > 0);
		std::cout << " ";
	}
	std::cout << ": " << val << " as binary\n";
}