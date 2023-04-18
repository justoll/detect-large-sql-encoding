#ifdef __cplusplus
#include <string.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <unicode/ucnv.h>
#include <unicode/ucsdet.h>
#include <unicode/uclean.h>
extern "C" { //using these C-functions in C++
    const char** getLanguages();
    size_t getCountLanguages();
    bool isUTF8(FILE* fileIn, float tolerance);
    const char* detectSingleByteKnownLanguage(FILE* fileIn, int languageNumber);
    const char* detectFarEastSafeMultibyte(FILE* fileIn, float tolerance);
    const char* detectEncodingOtherLanguages(FILE* fileIn, int languageNumber);
    const char* detectEncodingUnknownLanguage(FILE* fileIn);
    void clean(FILE* fileIn, FILE* fileCleaned);
    void initDetectSQLEncoding();
    const char* highlightNonASCII(const char* input);
    bool encodingIsFarEastSafeMultibyte(const char* encoding);
}
#endif
#define FILENAMECLEANED "cleaned.txt"
/*
 Function:  manualCheck
 --------------------
 fileIn: file to be checked
 encoding: encoding to be checked

 returns: true, if the manual check gave a positive result
 */
bool manualCheck(FILE* fileIn, const char* encoding)
{
    size_t level = 0; //higher level->more and longer examples
    fseek(fileIn, 0, SEEK_END); //find end of file
    size_t size = ftell(fileIn) + (size_t)1; //size of file 
    fseek(fileIn, 0, SEEK_SET); //reset to begin
    size_t origin;
    size_t step;
    size_t countSteps;
    
    while (true)
    {
        fseek(fileIn, 0, SEEK_SET); //go to begin of file
        switch (level)
        {
        case 0: //higher level->more and longer examples
            step = 1000; //length of an example
            countSteps = 3; //example count
            break;
        case 1:
            step = 3000;
            countSteps = 5;
            break;
        case 2:
            step = 5000;
            countSteps = 11;
            break;
        case 3:
            step = 8000;
            countSteps = 13;
            break;
        case 4:
            step = size; //example is the whole file
            countSteps = 1;
            break;
        default:
            exit(1); //there cant be more than 4 levels
            break;
        }
        origin = 0; //begin of first example
        if (step * countSteps > size) //examples longer than file ?
        {
            step = size; //example length = size
            countSteps = 1; //example count = 1
            level = 4; //jump to last level
        }
        
        for (size_t i = 0; i < countSteps; i++) //for every example
        {
            if (origin + step > size) 
            {
                origin = size - step; //prevent reading after the end of the file
            }
            fseek(fileIn, origin, SEEK_SET); //reset to begin
            char* excerpt = (char*)calloc((step + 1),sizeof(char));
            if (!excerpt)
            {
                std::cout << "Memory allocation failed!" << std::endl;
                exit(1);
            }
            size_t result = fread(excerpt, sizeof(char), step - 1, fileIn); //read example
            if (result != step-1) { fputs("Reading error", stderr); exit(3); }
            excerpt[step] = '\0'; //End of C-String
            UErrorCode uerr = U_ZERO_ERROR;
            char* excerptConverted = (char*)calloc((step + 1) * 8, sizeof(char));
            if (!excerptConverted)
            {
                std::cout << "Memory allocation failed!" << std::endl;
                free(excerpt);
                exit(1);
            }
            UConverter* uconv = ucnv_open(encoding, &uerr);
            if (U_FAILURE(uerr)) {
                std::cout << "Error loading ICU Converter!" << std::endl;
                free(excerpt);
                free(excerptConverted);
                exit(1);
            }
            ucnv_toAlgorithmic(UCNV_UTF8, uconv, excerptConverted, sizeof(char) * (step + 1)*3, excerpt, strlen(excerpt), &uerr); //convert to UTF-8
            const char* excerptConvertedHighlighted = highlightNonASCII(excerptConverted); //insert control chars to highlight the interesting parts(non - ASCII)
            std::cout << std::endl << std::endl << "Example " << i + 1 << ":\n" << excerptConvertedHighlighted << std::endl << std::endl;
            ucnv_close(uconv);
            free(excerpt);
            free(excerptConverted);
            origin += size / countSteps; //go to begin of next example
        }
        while (true) //until answer is given
        {
            char input;
            if (level >= 4) //final level?
            {
                std::cout << "The complete file has already been displayed! Please choose (y) or (n)\n";
                std::cout << "Has the Text been decoded in the correct way? Have a look at the \033[1;41mmarked\033[0m parts!\n(y)es, it is correct\n(n)o, there are errors\n";
            }
            else
            {
                std::cout << "Has the Text been decoded in the correct way? Have a look at the \033[1;41mmarked\033[0m parts!\n(y)es, it is correct\n(n)o, there are errors\n(m)ore examples are needed\n";
            }
            std::cin >> input;
            if (input == 'y') //yes, correct
                return true;
            if (input == 'n') //no, errors
                return false;
            if (input == 'm' && level < 4) //more examples needed. Only possible before final level
            {
                level++;
                break;
            }
        }
    }
    return false;
}
/*
 Function:  cleanSQLFile
 --------------------
 fileNameIn: filename of the file to be cleaned
 fileNameCleaned: [output by pointer] cleaned file
 */
void cleanSQLFile(char* fileNameIn, const char* fileNameCleaned)
{
    FILE* fileIn;
    FILE* fileCleaned;
    fileIn = fopen(fileNameIn, "rb");
    if (!fileIn)
    {
        std::cout << "Reading SQL-File failed!" << std::endl;
        exit(1);
    }
    fileCleaned = fopen(fileNameCleaned, "wb");
    if (!fileCleaned)
    {
        std::cout << "Writing cleaned file failed!" << std::endl;
        fclose(fileIn);
        exit(1);
    }
    clean(fileIn, fileCleaned);
    fclose(fileIn);
    fclose(fileCleaned);
}
/*
 Function:  detectEncoding
 --------------------
 fileNameIn: filename of the file to be detected

 returns: detected encoding
 */
const char* detectEncoding(const char* fileNameIn)
{
    bool repeating = true;
    const char** languages = getLanguages();
    size_t countLanguages = getCountLanguages();
    const char* confirmedEncoding = NULL;
    while (repeating) //until the user found an encoding or gave up
    {
        FILE* fileIn = fopen(fileNameIn, "rb");
        if (!fileIn)
        {
            std::cout << "Error reading file " << fileNameIn << std::endl;
            exit(1);
        }
        std::vector<const char*> encodings = {}; //list for possible encodings - first has highest probability
        if (isUTF8(fileIn, 0.05f))
        {
            encodings.push_back("UTF-8");
        }
        else
        {
            encodings.push_back(detectFarEastSafeMultibyte(fileIn, 0.01));
            if (!strcmp(encodings.back(), "euc"))
            {
                std::cout << "The Encoding is EUC-compatible. It is either GB 18030, EUC-JP or EUC-KR" << std::endl;
                encodings.pop_back();
                encodings.push_back("euc-kr"); //it has to be one of these 3 encodings
                encodings.push_back("euc-jp");
                encodings.push_back("gb18030");
                encodings.push_back("unknown"); //more investigations to do, we don't know yet which of the 3 it is
            }
            else if (!strcmp(encodings.back(), "unknown"))
            {
                std::cout << "The encoding is likely not UTF-8, ISO-2022, Shift_JIS or EUC-JP." << std::endl;
            }
            if (!strcmp(encodings.back(), "unknown")) //not far east safe multibyte?
            {
                encodings.pop_back();
                while (true)
                {
                    char input;
                    std::cout << "Do you want to specify a language (y or n)?" << std::endl;
                    std::cin >> input;
                    if (input == 'y')
                    {
                        std::cout << "Choose one of the following languages:" << std::endl;
                        for (size_t i = 0; i < countLanguages; i++) //print all languages with numbers
                        {
                            std::cout << i << ": " << languages[i] << std::endl;
                        }
                        int languageNumber;
                        std::cout << "Nr: ";
                        std::cin >> languageNumber;
                        const char* encodingKnownLanguage = detectSingleByteKnownLanguage(fileIn, languageNumber);
                        int j = 0;
                        bool alreadyExists = false;
                        
                        for (const char* encoding : encodings) //for each encoding guessed yet
                        {
                            if (!strcmp(encodingKnownLanguage, encoding))
                            {
                                encodings.erase(encodings.begin() + j); //move to the pole position of the encoding list
                                encodings.insert(encodings.begin(), encodingKnownLanguage);
                                alreadyExists = true;
                                break;
                            }
                            j++;
                        }
                        if (!alreadyExists)
                        {
                            encodings.push_back(encodingKnownLanguage); //add it to the end of the list
                        }
                        if (!strcmp(encodings.back(), "unknown"))
                        {
                            std::cout << "Language is not supported or it may be wrong, using language independent algorithm..." << std::endl;
                            encodings.push_back(detectEncodingOtherLanguages(fileIn, languageNumber));
                        }
                        const char* encodingUnknownLanguage = detectEncodingUnknownLanguage(fileIn);
                        j = 0;
                        alreadyExists = false;
                        for (const char* encoding : encodings) //for each encoding guessed yet
                        {
                            if (!strcmp(encodingUnknownLanguage, encoding))
                            {
                                encodings.erase(encodings.begin() + j); //move to the pole position of the encoding list
                                encodings.insert(encodings.begin(), encodingUnknownLanguage);
                                alreadyExists = true;
                                break;
                            }
                            j++;
                        }
                        if (!alreadyExists)
                        {
                            encodings.push_back(encodingUnknownLanguage); //add it to the end of the list
                        }
                        break;
                    }
                    if (input == 'n')
                    {
                        bool alreadyExists = false;
                        const char* encodingUnknownLanguage = detectEncodingUnknownLanguage(fileIn);
                        int j = 0;
                        for (const char* encoding : encodings) //for each encoding guessed yet
                        {
                            if (!strcmp(encodingUnknownLanguage, encoding))
                            {
                                encodings.erase(encodings.begin() + j); //move to the pole position of the encoding list
                                encodings.insert(encodings.begin(), encodingUnknownLanguage);
                                alreadyExists = true;
                                break;
                            }
                            j++;
                        }
                        if (!alreadyExists)
                        {
                            encodings.push_back(encodingUnknownLanguage); //add it to the end of the list
                        }
                        break;
                    }
                }
            }
        }
        for (const char* encoding : encodings) //for each encoding guessed yet
        {
            std::cout << "Encoding: \u001b[7m" << encoding << "\u001b[0m\n";
        }
        fileIn = fopen(fileNameIn, "rb");
        confirmedEncoding = NULL; //nothing confirmed yet
        for (const char* encoding : encodings) //for each encoding guessed yet
        {
            if (strcmp(encoding, "unknown"))
            {
                std::cout << "Manual check of \u001b[7m" << encoding << "\u001b[0m is going to be performed...\n";
                std::cout << "Press Enter to continue!" << std::endl;
                std::cin.ignore(); //Wait for enter
                if (manualCheck(fileIn, encoding)) 
                {
                    confirmedEncoding = encoding;
                    break;
                }
            }
        }
        if (confirmedEncoding == NULL)
        {
            if (!strcmp(encodings.front(), "UTF-8") || encodingIsFarEastSafeMultibyte(encodings.front()))
            {
                std::cout << "The detection of \u001b[7m" << encodings.front() << "\u001b[0m has been safe! The last encoding was done using this method, another use of this program or other detection-tools will not give better results. Maybe the file has been encoded multiple times. A manual investigation by a specialist is required." << std::endl;
                repeating = false; //no retry useful and allowed
            }
            else
            {
                std::cout << "The detection has failed. Maybe another try using other parameters (e.g. another language, unknown language) will produce a result. If not, a review by a specialist is required." << std::endl;
                while (true) //retry may be useful
                {
                    char input;
                    std::cout << "Do you want to try it again? (y or n)" << std::endl;
                    std::cin >> input;
                    if (input == 'y')
                    {
                        break; //back to main while loop
                    }
                    if (input == 'n')
                    {
                        std::cout << "Encoding could not be found" << std::endl;
                        repeating = false; //break main while loop
                        break;
                    }
                }
            }
        }
        else
        {
            repeating = false;
        }
        fclose(fileIn);
    }
    //for (size_t i = 0; i < countLanguages; i++)
    //{
   //    free(languages[i]);
    //}
    free(languages);
    return confirmedEncoding;
}

/*
 Function:  main
 --------------------
 argc: count of arguments
 argv: array of arguments

 returns: not zero if there are errors
 */
int main(int argc, char* argv[])
{
    if (argc <= 1)
    {
        std::cout << "Please specify the SQL-file!" << std::endl;
        return 1;
    }
    initDetectSQLEncoding();
    cleanSQLFile(argv[1], FILENAMECLEANED);
    const char* confirmedEncoding = detectEncoding(FILENAMECLEANED);
    if (confirmedEncoding == NULL) //detection failed?
    {
        return 0;
    }
    std::cout << "The detection of \u001b[7m" << confirmedEncoding << "\u001b[0m has been confirmed!" << std::endl;
}