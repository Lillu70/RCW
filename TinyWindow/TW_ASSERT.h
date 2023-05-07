#pragma once

#define CNB *((int*)0) = 666


#ifdef _DEBUG
#define TW_ASSERT(stament) if(!(stament)) CNB;
#else
#define TW_ASSERT(stament)
#endif // _DEBUG

#define TW_TERMINATE CNB;