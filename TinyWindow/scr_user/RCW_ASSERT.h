#pragma once

#include <stdlib.h>


#ifdef _DEBUG
#define ASSERT(stament) if(!(stament)) abort();
#else
#define ASSERT(stament)
#endif // _DEBUG
