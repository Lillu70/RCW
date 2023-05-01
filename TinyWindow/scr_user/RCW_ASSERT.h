#pragma once

inline void abort() 
{ 
	int* ptr = 0;
	*ptr = 0;
}

#ifdef _DEBUG
#define ASSERT(stament) if(!(stament)) abort();
#else
#define ASSERT(stament)
#endif // _DEBUG
