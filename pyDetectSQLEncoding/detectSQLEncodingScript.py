#!/usr/bin/python3
import detectsqlencoding
import sys
import os

FILENAMECLEANED = "cleaned.txt"

def manualCheck(filename, fileEncoding):
    """
    filename: filename of file to be checked
    encoding: encoding to be checked

    returns: true, if the manual check gave a positive result
    """
    level = 0 #higher level->more and longer examples
    size = os.path.getsize(filename) #filesize
    step = 0 #length of an example
    countSteps = 0 #example count
    print("Filesize: "+str(size))
    with open(filename, "rb") as f: #read binary
        while True:
            f.seek(0) #go to begin of file
            if level == 0: #higher level->more and longer examples
                step = 1000 #length of an example
                countSteps = 3 #example count
            elif level == 1:
                step = 3000
                countSteps = 5
            elif level == 2:
                step = 5000
                countSteps = 11
            elif level == 3:
                step = 8000
                countSteps = 13
            elif level == 4: #example is the whole file
                step = size 
                countSteps = 1
            else:
                print("Error!") #there cant be more than 4 levels
                break
            origin = 0 #begin of first example
            if step * countSteps > size: #examples longer than file?
                step = size #example length = size
                countSteps = 1 #example count = 1
                level = 4 #jump to last level
            for i in range(0, countSteps): #for every example
                if origin+step>size:
                    origin = size-step
                print("Origin: "+str(origin)+" size "+str(size)+" step "+str(step))
                f.seek(origin) #go to begin of example
                excerpt = f.read(step-1) #read example
                excerptConvertedHighlighted = detectsqlencoding.highlightNonASCII(excerpt).decode(fileEncoding, errors="ignore") #insert control chars to highlight the interesting parts (non-ASCII)

                print("\n\nExample #" + str(i+1) + ":\n" + excerptConvertedHighlighted)
                origin += size // countSteps #go to begin of next example
            while True: #until answer is given
                if level>=4: #final level?
                    print("\nThe complete file has been displayed!")
                    print("Has the Text been decoded in the correct way? Have a look at the \033[1;41mmarked\033[0m parts!\n(y)es, it is correct\n(n)o, there are errors")
                else:
                    print("Has the Text been decoded in the correct way? Have a look at the \033[1;41mmarked\033[0m parts!\n(y)es, it is correct\n(n)o, there are errors\n(m)ore examples are needed\n")
                answer = input("")
                if answer == "y": #yes, correct
                    return True;
                if answer == "n": #no, errors
                    return False;
                if answer== "m" and level<4: #more examples needed. Only possible before final level
                    level += 1
                    break
    return False
def detectEncoding(filename):
    """
    fileNameIn: filename of the textfile to be detected

    returns: detected encoding
    """
    languages = detectsqlencoding.getLanguages() #Binary, has to be decoded to string with utf-8
    repeating = True
    
    while repeating: #until the user found an encoding or gave up
        encodings = [] #list for possible encodings - first has highest probability
        if detectsqlencoding.isUTF8(filename, 0.05): #UTF-8?
            encodings.append('UTF-8') #One and only possible encoding
        else:
            encodings.append(detectsqlencoding.detectFarEastSafeMultibyte(filename, 0.01).decode('utf-8').lower())
            print(encodings)
            if encodings[-1]=="euc": #EUC-compatible?
                print("The Encoding is EUC-compatible. It is either GB 18030, EUC-JP or EUC-KR")
                encodings.pop()
                encodings.append("euc-kr") #it has to be one of these 3 encodings
                encodings.append("euc-jp")
                encodings.append("gb18030")
                encodings.append("unknown") #more investigations to do, we don't know yet which of the 3 it is
            elif encodings[-1]=="unknown": #no safe detection method had any result?
                print("The encoding is likely not UTF-8, ISO-2022, Shift_JIS or EUC.") 
            if encodings[-1]=="unknown": #now the encoding will be guessed and not safely detected
                encodings.pop()
                
                while True:
                    answer=input("Do you want to specify a language (y or n)")
                    if answer == 'y':
                        print("Choose one of the following langugages:")
                        for i in range(len(languages)): #print all languages with numbers
                            print(str(i)+": "+languages[i].decode('utf-8'))
                        try:
                            languageNumber = int(input("Nr: "))
                        except ValueError:
                            print("Error: This is not a number!")
                            continue
                        print(encodings)
                        encodingKnownLanguage = detectsqlencoding.detectSingleByteKnownLanguage(filename, languageNumber).decode('utf-8').lower() #detect using the language-dependent method
                        if encodingKnownLanguage in encodings: #has the encoding already been found before?
                            encodings.remove(encodingKnownLanguage)
                            encodings.insert(0, encodingKnownLanguage) #move to the pole position of the encoding list
                            print(encodings)
                        else: #not detected yet
                            encodings.append(encodingKnownLanguage) #add it to the end of the list
                            print(encodings)
                        if encodings[-1] == "unknown": #language-dependent method didn't give a result
                            encodings.pop()
                            print("Language is not supported or it may be wrong, using language independent algorithm...")
                            encodings.append(detectsqlencoding.detectEncodingOtherLanguages(filename, languageNumber).decode('utf-8').lower()) #using fallback method, ICU
                        encodingUnknownLanguage = detectsqlencoding.detectEncodingUnknownLanguage(filename).decode('utf-8').lower() #additional: using the language-independent method
                        if encodingUnknownLanguage in encodings: #has the encoding already been found before?
                            encodings.remove(encodingUnknownLanguage)
                            encodings.insert(0, encodingUnknownLanguage) #move to the pole position of the encoding list
                        else: #not detected yet
                            encodings.append(encodingUnknownLanguage) #add it to the end of the list
                        break
                    elif answer == 'n':
                        encodingUnknownLanguage = detectsqlencoding.detectEncodingUnknownLanguage(filename).decode('utf-8').lower()#using the language-independent method
                        if encodingUnknownLanguage in encodings:#has the encoding already been found before?
                            encodings.remove(encodingUnknownLanguage)
                            encodings.insert(0, encodingUnknownLanguage)#move to the pole position of the encoding list
                        else: #not detected yet
                            encodings.append(encodingUnknownLanguage)#add it to the end of the list
                        break
        for i in range(len(encodings)): #print all possibilities with ranking
            print("Encoding #"+str(i+1)+": \u001b[7m"+encodings[i]+"\u001b[0m")
        confirmedEncoding = None #nothing confirmed yet
        for encoding in encodings: #for each possible encoding
            print("Manual check of \u001b[7m"+encoding+"\u001b[0m is going to be performed...")
            input("Press Enter to continue!")
            if manualCheck(filename, encoding):#perform manual check
                confirmedEncoding = encoding #confirmed!
                break #break for loop, other encodings not interesting anymore
        if confirmedEncoding==None: #nothing confirmed
            if encodings[0] == "UTF-8" or detectsqlencoding.encodingIsFarEastSafeMultibyte(encoding[0]):
                print("The detection of \u001b[7m"+encodings[0]+"\u001b[0m has been safe! The last encoding was done using this method, another use of this program or other detection-tools will not give better results. Maybe the file has been encoded multiple times. A manual investigation by a specialist is required.")
                repeating = False #no retry useful and allowed
            else:
                print("The detection has failed. Maybe another try using other parameters (e.g. another language, unknown language) will produce a result. If not, a review by a specialist is required.")
                while True: #retry may be useful
                    answer=input("Do you want to try it again? (y or n)")
                    if answer=='y':
                        break
                    if answer=='n':
                        repeating=False
                        break
        else:
            repeating=False #finished!
    return confirmedEncoding

if len(sys.argv) <= 1: # no arguments?
    print("Please specify a SQL-File")
    exit()
detectsqlencoding.init() # initialize module
detectsqlencoding.clean(str(sys.argv[1]), FILENAMECLEANED) # clean SQL File
encoding = detectEncoding(FILENAMECLEANED) 
if encoding is not None:
    print("The detection of \u001b[7m"+encoding+"\u001b[0m has been confirmed!")
