#ifndef _DETECT_TEXT_ENCODING
#define _DETECT_TEXT_ENCODING

#include <stdio.h>
#include <unicode/ucnv.h>
#include <unicode/ucsdet.h>
#include <unicode/uclean.h>
#include <enca.h>

#define ADDITIONALLANGUAGES {"dutch", "english", "german", "finnish", "french", "irish", "italian", "portuguese", "spanish", "swedish", "norwegian", "arabic", "greek", "romanian", "thai", "hebrew", "turkish", "other"}
#define COUNTADDITIONALLANGUAGES 18

#define LANGUAGENAMESIZE 20

extern void initDetectTextEncoding();
extern const char** getLanguages();
extern size_t getCountLanguages();
extern _Bool isUTF8(FILE* fileIn, float tolerance);
extern const char* detectSingleByteKnownLanguage(FILE* fileIn, size_t languageNumber);
extern const char* detectFarEastSafeMultibyte(FILE* fileIn, float tolerance);
extern const char* detectEncodingOtherLanguages(FILE* fileIn, size_t languageNumber);
extern const char* detectEncodingUnknownLanguage(FILE* fileIn);
extern _Bool encodingIsFarEastSafeMultibyte(const char* encoding);
extern const char* detectEnca(FILE* fileIn, size_t languageNumber);
#endif