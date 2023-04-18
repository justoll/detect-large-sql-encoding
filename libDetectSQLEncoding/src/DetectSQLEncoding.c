#include "DetectSQLEncoding.h"
/*
 Function:  initDetectSQLEncoding
 --------------------
 calls the initializers for cleaning and detecting
 to initialize variables
 */
void initDetectSQLEncoding()
{
	initCleanSQL();
	initDetectTextEncoding();
}
/*
 Function:  highlightNonASCII
 --------------------
 input: text to be highlighted

 returns: highlighted text as const char*
 */
const char* highlightNonASCII(const char* input)
{
    size_t countChars = 0;
    size_t countNonASCIIParts = 0;
    bool inNonASCII = false;
    while (input[countChars] != '\0') //until end of string is reached
    {
        if ((unsigned char)input[countChars] >= 0x80) //a non-ASCII byte has been found
        {
            inNonASCII = true;
        }
        else if (inNonASCII)
        {
            inNonASCII = false; //byte before was non-ASCII but this is ASCII
            countNonASCIIParts++; //count of the parts to be higlighted
        }
        countChars++;
    } 
    char* highlighted = (char*)calloc(countChars + countNonASCIIParts * 12 + 10, sizeof(char)); //allocate memory for output considering space for the control characters for highlighting
    if (!highlighted)
    {
        printf("Memory allocation failed!\n");
        return NULL;
    }
    inNonASCII = false;
    size_t charOutput = 0;
    for (size_t i = 0; i < countChars; i++)
    {
        if ((unsigned char)input[i] >= 0x80) //a non-ASCII byte has been found
        {
            if (!inNonASCII) //byte before was ASCII, this is non-ASCII
            {
                inNonASCII = true;
                highlighted[charOutput] = '\033'; //begin highligting (fat white text on red)
                charOutput++;
                highlighted[charOutput] = '[';
                charOutput++;
                highlighted[charOutput] = '1';
                charOutput++;
                highlighted[charOutput] = ';';
                charOutput++;
                highlighted[charOutput] = '4';
                charOutput++;
                highlighted[charOutput] = '1';
                charOutput++;
                highlighted[charOutput] = 'm';
                charOutput++;
            }
        }
        else if (inNonASCII)  //byte before was non-ASCII, this is ASCII
        {
            inNonASCII = false;
            highlighted[charOutput] = '\033'; //end highligting (white text on black)
            charOutput++;
            highlighted[charOutput] = '[';
            charOutput++;
            highlighted[charOutput] = '0';
            charOutput++;
            highlighted[charOutput] = 'm';
            charOutput++;

        }
        highlighted[charOutput] = input[i]; //add byte to string
        charOutput++;
    }
    highlighted[charOutput] = '\033'; //stop highlighting at end of string
    charOutput++;
    highlighted[charOutput] = '[';
    charOutput++;
    highlighted[charOutput] = '0';
    charOutput++;
    highlighted[charOutput] = 'm';
    charOutput++;
    highlighted[charOutput] = '\0';
    charOutput++;
    return highlighted;
}