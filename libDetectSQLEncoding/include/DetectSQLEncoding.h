#ifndef _DETECT_SQL_ENCODING
#define _DETECT_SQL_ENCODING

#include "CleanSQL.h"
#include "DetectTextEncoding.h"

extern void initDetectSQLEncoding();
extern const char* highlightNonASCII(const char* input);

#endif