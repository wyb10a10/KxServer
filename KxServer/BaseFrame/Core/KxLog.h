#ifndef __KX_LOG_H__
#define __KX_LOG_H__

#include <stdio.h>

#include "ICore.h"

#define MAX_LEN 256

inline void KxLog(const char* format, ...)
{
    char szBuf[MAX_LEN];

    va_list ap;
    va_start(ap, format);
    vsnprintf_s(szBuf, MAX_LEN, MAX_LEN, format, ap);
    va_end(ap);
    printf("%s\n", szBuf);
}

#endif
