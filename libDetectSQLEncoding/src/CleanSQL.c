#include "CleanSQL.h"
#define TRUE 1
#define FALSE 0
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/*
 Function:  initCleanSQL
 --------------------
 initializes variables for cleaning SQL-Files
 for now, this function does nothing
 */
void initCleanSQL()
{

}
/*
 Function:  clean
 --------------------
 fileIn: Pointer to the file to read (input)
 fileCleaned: output-file (pure text), containing the pure content after execution
  
 cleanes a given SQL-file by removing parts 
 which may cause negative effects for detection.
 */
void clean(FILE* fileIn, FILE* fileCleaned)
{
    unsigned char byte;
    _Bool isString = FALSE;
    unsigned int countBackslashes = 0;
    _Bool relevant = TRUE;
    _Bool specialCharHere = FALSE;
    char line[10000];
    int i = 0;
    while (!feof(fileIn))
    {
        fread(&byte, sizeof(char), 1, fileIn); //read one Byte
        if (isString) //are we in a string?
        {

            if (byte == 0x27) //0x27 = '
            {
                if (countBackslashes % 2 == 0) //Apostroph not behind \ ?
                {
                    isString = FALSE;
                    if (relevant && specialCharHere)
                    {
                        line[i] = '\n'; //new line
                        line[i + 1] = '\0'; //end of C-String
                        //printf("%s", line);
                       
                        fwrite(line, sizeof(char), i + 1, fileCleaned); //write string
                    }
                    i = 0; //new string
                    countBackslashes = 0;
                    relevant = TRUE; //Reset  New String is relevant at the beginning
                    specialCharHere = FALSE; //Reset  New String has no Characters yet
                    continue;
                }
            }
            else if (byte == 0x5C) //0x5C = Backslash
            {
                countBackslashes++;
                relevant = FALSE;
            }
            else
            {
                countBackslashes = 0;
            }
            if (byte == '<' || byte == '>' || byte == '#' || byte == '&') //Forbidden char?
            {
                relevant = FALSE;
            }
            if (i < 9998) //only the first 10.000 chars of a string as output
            {
                line[i] = byte;
                i++;
            }
            if (byte >= 0x80 || byte == 0x1b) //Non-ASCII or ESC?
            {
                specialCharHere = TRUE;
            }
        }
        else
        {
            if (byte == 0x27)
            {
                isString = TRUE;
            }
        }
    }
}