#ifndef _TEARA_LIB_UTILS_DEBUG_H_
#define _TEARA_LIB_UTILS_DEBUG_H_

#if TEARA_DEBUG
    #define Assert(Expression)  ((Expression) ? 1 : *((int*)0));
#else
    #define Assert(Expression)
#endif

#endif