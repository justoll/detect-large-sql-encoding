#include "DetectTextEncoding.h"
#include <stdio.h>
#include <string.h>
#include <unicode/ucnv.h>
#include <unicode/ucsdet.h>
#include <unicode/uclean.h>
#include <enca.h>

#define TRUE 1
#define FALSE 0

const char** encaLanguages;
size_t countEncaLanguages;
size_t countLanguages=0;
const char** languages = NULL;
/*
 Function:  getLanguages
 --------------------
 returns: all initialized languages including enca
 before using getLanguages, initDetectTextEncoding is requiered.
 otherwise returns NULL
 */
const char** getLanguages()
{
    return languages;
}
/*
 Function:  getCountLanguages
 --------------------
 returns: number of all initialized languages including enca
 before using getCountLanguages, initDetectTextEncoding is requiered.
 otherwise returns 0
*/
size_t getCountLanguages()
{
    return countLanguages;
}
/*
 Function:  initDetectTextEncoding
 --------------------
 initializes the detection by generating
 the array of languages. Combining enca-languages 
 with built-in-languages
 */
void initDetectTextEncoding()
{
    const char* additionalLanguages[COUNTADDITIONALLANGUAGES] = ADDITIONALLANGUAGES; //init additional languages
    encaLanguages = enca_get_languages(&countEncaLanguages); //built-in-languages enca
    countEncaLanguages--; //last one 'none' not needed
    countLanguages = countEncaLanguages + COUNTADDITIONALLANGUAGES; //total languages
    languages = (const char**)malloc(countLanguages * sizeof(const char*)); //memory allocate dynamic string-array
    if (languages == NULL)
    {
        printf("memory allocation failed!\n");
        return;
    }
    for (size_t i = 0; i < countLanguages; i++)
    {
        if (i < countEncaLanguages)
        {
            languages[i] = enca_language_english_name(encaLanguages[i]); //add an enca-language
        }
        else
        {
            languages[i] = additionalLanguages[i - countEncaLanguages]; //add an additional, built-in-language
        }
    }
}
/*
 Function:  isUTF8
 --------------------
 fileIn: file to be investigated
 tolerance: value of tolerance for UTF-8 detection. Should be >=0 and <1

 returns: true, if the file is encoded in UTF-8 
          considering the tolerance. Otherwise: false
 */
_Bool isUTF8(FILE* fileIn, float tolerance)
{

    fseek(fileIn, 0, SEEK_SET); //go to file-beginning
    unsigned char byte = 0;
    unsigned int countBytes = 0;   //Number of bytes in character excluding Startbyte
    unsigned int countCorrect = 0; //Number of correct bytes
    unsigned int countErrors = 0; //Number of error-bytes
    while (!feof(fileIn)) //loops through every byte
    {
        fread(&byte, sizeof(char), 1, fileIn); //read one byte
        if (byte == '\n')
        {
            countBytes = 0; //reset number of bytes new line
        }
        if (byte >= 0x80) //begins with 1 - Non-ASCII?
        {
            if ((byte & 0b11000000) == 0b11000000) //Startbyte?
            {
                if (countBytes > 0)
                {
                    countErrors++; //there should be a following byte, but it is a startbyte!
                }
                else
                {
                    countCorrect++;
                }
                countBytes = 1; //one or more following Bytes.
                if (byte & 0b00100000) //two ore more following Bytes?
                {
                    countBytes = 2;
                    if (byte & 0b00010000) //three ore more following Bytes?
                    {
                        countBytes = 3;
                        if (byte & 0b00001000) //four...
                        {
                            countBytes = 4;
                            if (byte & 0b00000100)
                            {
                                countBytes = 5;
                                if (byte & 0b00000010)
                                {
                                    countBytes = 6;
                                    if (byte & 0b00000001)
                                    {
                                        countBytes = 7;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else if ((byte & 0b11000000) == 0b10000000) //Following byte?
            {
                if (countBytes)
                {
                    countCorrect++; //following byte expected, following byte has come
                    countBytes--; //reduce expected number of following bytes to come
                }
                else
                {
                    countErrors++; //there should be no following byte
                }

            }
            else //this is NO startbyte and NO following byte but begins with '1' - Error!
            {
                countErrors++;
            }
        }
    }
    if (countErrors < (countCorrect + countErrors) * tolerance) //criterion for tolerance fulfilled?
        return TRUE; //UTF-8
    else
        return FALSE; //not UTF-8
}
/*
 Function:  detectEnca
 --------------------
 fileIn: file to be investigated
 languageNumber: number of language in languages

 returns: C-string containing encoding detected by enca using 
          the given language. If detection fails: "unknown"
 */
const char* detectEnca(FILE* fileIn, size_t languageNumber)
{

    fseek(fileIn, 0, SEEK_END); //find end of file
    long size = ftell(fileIn) + 1; //size of file 
    fseek(fileIn, 0, SEEK_SET); //reset to begin
    unsigned char* content = (unsigned char*)malloc(sizeof(unsigned char) * size); //request memory for file loading from heap
    if (content) //memory correct allocated?
    {
        fread(content, 1, size, fileIn); //read whole file to buffer
        content[size - 1] = '\0'; //terminate C-string
        EncaAnalyser encaAnalyser;
        if(languageNumber < countEncaLanguages) //is the language an enca-language?
            encaAnalyser = enca_analyser_alloc(encaLanguages[languageNumber]); //create analyser with language
        else
            encaAnalyser = enca_analyser_alloc("__"); //create analyser without language (only UTF-8 detection)
        if (encaAnalyser)
        {
            EncaEncoding encaEncoding = enca_analyse(encaAnalyser, content, size); //detection
            enca_analyser_free(encaAnalyser); //delete analyser
            free(content);
            const char* str = enca_charset_name(encaEncoding.charset, ENCA_NAME_STYLE_MIME); //get MIME-encoding-name
            if (str && strcmp(str, "unknown"))
            {
                if (!strcmp(str, "GB2312")) //enca returns gb18030 (encoding) as gb2312 (charset)
                {
                    return "gb18030"; //return the encoding
                }
                return str;
            }
            else
            {
                return "unknown"; //detection using enca failed
            }
        }
        else
        {
            free(content);
            size_t a;
            const char ** langs = enca_get_languages(&a);
            
            return langs[a-2]; //enca languages without the last two ('none')
        }
    }
    else
    {
        printf("memory allocation failed!\n");
        return "error"; 
    }
}
/*
 Function:  detectSingleByteKnownLanguage
 --------------------
 fileIn: file to be investigated
 languageNumber: number of language in languages

 returns: C-string containing encoding guessed by enca or having a look at the language in general.
 If detection fails: "unknown"
 */
const char* detectSingleByteKnownLanguage(FILE* fileIn, size_t languageNumber)
{
    if (languageNumber < countEncaLanguages)
        return(detectEnca(fileIn, languageNumber));
    languageNumber -= countEncaLanguages;
    switch (languageNumber)
    {
    case 0: //Dutch
    case 1: //English
    case 2: //German
    case 3: //Finnish
    case 4: //French
    case 5: //Irish
    case 6: //Italian
    case 7: //Portuguese
    case 8: //Spanish
    case 9: //Swedish
    case 10: //Norwegian
        return "windows-1252";
    case 11: //Arabic
        return "windows-1256";
    case 12: //Greek
        return "windows-1253";
    case 13: //Romanian
        return "unknown";
    case 14: //Thai
        return "windows-874";
    case 15: //Hebrew
        return "windows-1255";
    case 16: //Turkish
        return "windows-1254";
    case 17: //Other
    default:
        return "unknown";
    }
}
enum STATE
{
    NOCHARSET,
    ESCAPE,
    SECOND,
    THIRD,
    INASCII,
    INOTHERCHARSET
} state;
/*
 Function:  detectISO2022
 --------------------
 fileIn: file to be investigated
 tolerance: value of tolerance for ShiftHIS detection. Should be >=0 and <1

 returns: C-string containing encoding ISO-2022-XX if one of these encoding is detected (considering the tolerance)
 If not: "unknown"
 */
const char* detectISO2022(FILE* fileIn, float tolerance)
{
    fseek(fileIn, 0, SEEK_SET); //go to beginning of file
    char* iso2022Encoding = "ISO-2022";
    unsigned char second =' ';
    unsigned char third =' ';
    unsigned char byte = 0; //actual byte
    unsigned int countCorrect = 0; //Number of correct bytes
    unsigned int countErrors = 0; //Number of error-bytes
    state = NOCHARSET;
    while (!feof(fileIn)) //loops through every byte
    {
        fread(&byte, sizeof(char), 1, fileIn); //read one byte
        if (byte == '\n') //new line found
        {
            state = NOCHARSET; //reset state (no ESC found yet)
        }
        switch (state)
        {
        case NOCHARSET: //no ESC yet
            if (byte == 0x1B) //ESC-char found. Good indicator for ISO-2022 
            {
                countCorrect++;
                state = ESCAPE; //ESC has been found. Looking for more indicator-chars
            }
            else
            {
                if (byte >= 0x80) //bytes >=0x80 are not allowed in ISO-2022. Error-byte.
                {
                    countErrors++;
                }
            }
            break;
        case ESCAPE: //ESC has already been found
            if (byte == '(' || byte == '$' || byte == '.') //is after ESC an allowed char?
            {
                countCorrect++;
                state = SECOND; //ESC(first) and second byte have been found. Third byte has to be checked.
                second = byte; //save second byte for later
            }
            else
            {
                countErrors++; //a not ISO-2022 matching byte
            }
            break;
        case SECOND: //ESC and second byte have been found
            if (byte == 'B' || byte == 'J' || byte == '@' || byte == '(' || byte == 'A' || byte == 'F' || byte == 'I') 
            {
                countCorrect++; //third byte also matched ISO-2022
                third = byte; //save third byte for later
                if (second == '(' && third == 'B')
                {
                    state = INASCII; //the byte-combination guided to an ASCII-block
                }
                else if (second == '$' && third == '(')
                {
                    state = THIRD; //first three bytes have been found. Another fourth byte has to be checked
                }
                else
                {
                    state = INOTHERCHARSET; //the byte-combination guided to another, undefined charset
                }
            }
            else
            {
                countErrors++; //third byte did not match ISO-2022
            }
            break;
        case THIRD: //first three bytes have been found, but a fourth is expected
            if (byte == 'D' || byte == 'O' || byte == 'P' || byte == 'Q') //the 4-byte-combination indicates ISO-2022-JP
            {
                iso2022Encoding = "ISO-2022-JP";
                state = INOTHERCHARSET;
                countCorrect++;
            }
            else if (byte == 'C' && strcmp(iso2022Encoding, "ISO-2022-JP")) //the 4-byte-combination indicates ISO-2022-JP
            {
                iso2022Encoding = "ISO-2022-JP";
                state = INOTHERCHARSET;
                countCorrect++;
            }
            else if (byte == 'A' || byte == 'G' || byte == 'H' || byte == 'E' || byte == 'I' || byte == 'J' || byte == 'K' || byte == 'L' || byte == 'M')
            {
                iso2022Encoding = "ISO-2022-CN"; //the 4-byte-combination indicates ISO-2022-CN
                state = INOTHERCHARSET;
                countCorrect++;
            }
            else
            {
                state = NOCHARSET;
                countErrors++; //fourth byte did not match ISO-2022
            }
            break;
        case INASCII:
            if (byte < 0x80) 
            {
                countCorrect++;
            }
            else
            {
                countErrors++; //bytes >=0x80 are not allowed anywhere (7-bit-encoding)
            }
            break;
        case INOTHERCHARSET: //in any charset, waiting for the end (ESC-byte)
            countCorrect++;
            if (byte == 0x1B) //ESC
            {
                countCorrect++;
                state = ESCAPE; //charset ended. First byte found
            }
            break;
        default:
            break;
        }
    }
    if (countErrors <= (countCorrect + countErrors) * tolerance) //criterion for tolerance fulfilled?
        return iso2022Encoding;
    else
        return "unknown";
}
/*
 Function:  isShiftJIS
 --------------------
 fileIn: file to be investigated
 tolerance: value of tolerance for ShiftJIS detection. Should be >=0 and <1

 returns: TRUE, if given file is encoded in ShiftJIS. Otherwise: FALSE
 */
_Bool isShiftJIS(FILE* fileIn, float tolerance)
{
    fseek(fileIn, 0, SEEK_SET); //go to beginning of file
    unsigned char byte = 0; //actual byte
    unsigned int countCorrect = 0; //number of correct bytes
    unsigned int countErrors = 0; //number of error-bytes
    _Bool secondByte = FALSE; 
    while (!feof(fileIn)) //loops through every byte
    {
        fread(&byte, sizeof(char), 1, fileIn); //read one byte
        if (byte == '\n') //new line?
        {
            secondByte = FALSE; //always first byte in a new line
        }
        else if (!secondByte) //we are in a first byte
        {
            if (byte == 0x80 || byte == 0xA0 || byte >= 0xF0) //bytes not allowed as first bytes
            {
                countErrors++;
            }
            else if ((byte >= 0x81 && byte <= 0x9F) || (byte >= 0xE0 && byte <= 0xEF)) //bytes leading to a second byte
            {
                secondByte = TRUE;
            }
            else
            {
                continue; //ASCII. Not relevant for detection of ShiftJIS
            }
        }
        else //we are in a second byte
        {
            if (byte <= 0x3F || byte == 0x7F || byte >= 0xFD) //bytes not allowed as second bytes
            {
                countErrors++;
            }
            else
            {
                countCorrect++; //second byte has a correct value 
            }
            secondByte = FALSE; //after second byte, another first byte is expected
        }
    }
    if (countErrors <= (countCorrect + countErrors) * tolerance) //criterion for tolerance fulfilled?
        return TRUE;
    else
        return FALSE;

}
/*
 Function:  isEUC
 --------------------
 fileIn: file to be investigated
 tolerance: value of tolerance for EUC-detection. Should be >=0 and <1

 returns: TRUE, if given file is encoded in an EUC-encoding. Otherwise: FALSE
 */
_Bool isEUC(FILE* fileIn, float tolerance)
{
    fseek(fileIn, 0, SEEK_SET); //go to beginning of file
    unsigned char byte = 0;
    unsigned int countCorrect = 0; //number of correct bytes
    unsigned int countErrors = 1; //number of error-bytes. 1, otherwise texts without markers will be detected as EUC
    int bytesToCome = 0;
    while (!feof(fileIn)) //loops through every byte
    {
        fread(&byte, sizeof(char), 1, fileIn); //read one byte
        if (byte == '\n') //new line found
        {
            bytesToCome = 0; //reset
        }
        else if (!bytesToCome) //there are not more bytes expected. This is a first byte
        {
            if (byte < 0x80) //ASCII is not relevant for EUC-detection
            {
                continue;
            }
            else if (byte >= 0xA0) //bytes >=0xA0 are not relevant for EUC-detection 
            {
                continue;
            }
            else if (byte == 0x8F || byte == 0x8E) //markers for a 2-byte-char
            {
                bytesToCome = 2;
            }
            else if (byte >= 0x80) //other bytes >=0x80 and <0xA0 are not allowed as first byte
            {
                countErrors++;
            }
        }
        else //this is a not a first byte
        {
            if (byte < 0xA0) //only bytes >=0xA0 are allowed as following bytes
            {
                countErrors++;
            }
            else
            {
                countCorrect++;
                bytesToCome--; //one following byte has been found
            }
        }
    }
    if (countErrors <= (countCorrect + countErrors) * tolerance) //criterion for tolerance fulfilled?
        return TRUE;
    else
        return FALSE;

}
/*
 Function:  detectFarEastSafeMultibyte
 --------------------
 fileIn: file to be investigated
 tolerance: value of tolerance for the detections. Should be >=0 and <1

 returns: C-string containing encoding if one of the far-east-safe-multibyte-encodings is detected (considering the tolerance)
 If not: "unknown"
 */
const char* detectFarEastSafeMultibyte(FILE* fileIn, float tolerance)
{
    const char* encoding = detectISO2022(fileIn, tolerance);
    if (strcmp(encoding, "unknown"))
        return encoding;
    else if (isShiftJIS(fileIn, tolerance))
        return "Shift_JIS";
    else if (isEUC(fileIn, tolerance))
        return "EUC";
    else
        return "unknown";
}
/*
 Function:  encodingIsFarEastSafeMultibyte
 --------------------
 encoding: string to be checked

 returns: if a given string is equal to a far-east-safe-multibyte-encoding
 */
_Bool encodingIsFarEastSafeMultibyte(const char* encoding)
{
    if (strcmp(encoding, "Shift_JIS") && strcmp(encoding, "ISO-2022") && strcmp(encoding, "ISO-2022-JP") && strcmp(encoding, "ISO-2022-CN"))
        return false;
    return true;
}
struct DetectionResult
{
    const char* encoding;
    const char* language;
    const int32_t confidence; //in %
};
/*
 Function:  detectEncodingICU
 --------------------
 fileIn: file to be investigated

 returns: struct containing encoding, language and confidence detected by ICU
 */
struct DetectionResult detectEncodingICU(FILE* fileIn)
{
    UErrorCode uerr = U_ZERO_ERROR;
    fseek(fileIn, 0, SEEK_END); //find end of file
    long size = ftell(fileIn) + 1; //size of file 
    fseek(fileIn, 0, SEEK_SET); //reset to begin
    char* content = (char*)malloc(sizeof(char) * size); //request memory for file loading from heap
    if (content) //memory correct allocated?
    {
        fread(content, 1, size, fileIn); //read whole file to buffer
        content[size - 1] = '\0'; //end of C-string
        UCharsetDetector* ucd = ucsdet_open(&uerr); //ICU Detector
        ucsdet_setText(ucd, content, size - 1, &uerr);
        UCharsetMatch const* match = ucsdet_detect(ucd, &uerr); //find charset
        struct DetectionResult detectionResult = { ucsdet_getName(match, &uerr), ucsdet_getLanguage(match, &uerr), ucsdet_getConfidence(match, &uerr) };
        ucsdet_close(ucd);
        free(content); //free heap
        return detectionResult; //return struct
    }
    else
    {
        printf("Memory allocation failed!\n");
        struct DetectionResult detectionResult = { NULL, NULL, 0 };
        return detectionResult;//error in allocation
    }
}
/*
 Function:  detectEncodingOtherLanguages
 --------------------
 fileIn: file to be investigated
 languageNumber: number of language in languages

 returns: C-string containing encoding detected using
          the given language (fallback option: ICU).
 */
const char* detectEncodingOtherLanguages(FILE* fileIn, size_t languageNumber)
{
    const struct DetectionResult detectionResult = detectEncodingICU(fileIn);
    if (detectionResult.encoding)
    {
        return detectionResult.encoding;
    }
    else
    {
        return "Error";
    }
}
/*
 Function:  detectEncodingUnknownLanguage
 --------------------
 fileIn: file to be investigated

 returns: C-string containing encoding detected without knowing a language (fallback option: ICU).
 */
const char* detectEncodingUnknownLanguage(FILE* fileIn)
{
    const struct DetectionResult detectionResult = detectEncodingICU(fileIn);
    if (detectionResult.encoding)
    {
        return detectionResult.encoding;
    }
    else
    {
        return "Error";
    }
}
