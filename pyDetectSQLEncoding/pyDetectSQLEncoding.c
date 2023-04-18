#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <DetectSQLEncoding.h>
#define DETECTSQLENCODING_DOC "This module is for encoding-detection of SQL files."

static PyObject* DetectSQLEncodingError;
/*
 Function:  detectsqlencoding_init
 --------------------
 wrapper for initDetectSQLEncoding

 returns: none, in case of error another value
 */
static PyObject* detectsqlencoding_init(PyObject* self, PyObject* args)
{
    initDetectSQLEncoding();
    Py_RETURN_NONE;
}
/*
 Function:  detectsqlencoding_clean
 --------------------
 wrapper for clean
 filenameIn (in args): name of file to be investigated
 filenameCleaned (in args): name of cleaned output-file

 returns: none
 */
static PyObject* detectsqlencoding_clean(PyObject* self, PyObject* args)
{
    char* filenameIn;
    char* filenameCleaned;
    FILE* fileIn;
    FILE* fileCleaned;
    if (!PyArg_ParseTuple(args, "ss", &filenameIn, &filenameCleaned))
    {
        PyErr_SetString(DetectSQLEncodingError, "Parsing arguments failed. Check the argument list");
        return NULL;
    }
    fileIn = fopen(filenameIn, "rb");
    if (!fileIn)
    {
        PyErr_SetString(DetectSQLEncodingError, "Loading file failed");
        return NULL;
    }
    fileCleaned = fopen(filenameCleaned, "wb");
    if (!fileCleaned)
    {
        PyErr_SetString(DetectSQLEncodingError, "Loading output file failed");
        return NULL;
    }
    clean(fileIn, fileCleaned);
    fclose(fileIn);
    fclose(fileCleaned);
    Py_RETURN_NONE;
}
/*
 Function:  detectsqlencoding_detectSingleByteKnownLanguage
 --------------------
 wrapper for detectSingleByteKnownLanguage
 filenameIn (in args): name of file to be investigated
 language (in args): known language

 returns: detected single byte encoding using language dependent algorithm, "unknown" otherwise
 */
static PyObject* detectsqlencoding_detectSingleByteKnownLanguage(PyObject* self, PyObject* args)
{
    const char* filenameIn;
    const char* encoding;
    FILE* fileIn;
    int language;
    if (!PyArg_ParseTuple(args, "si", &filenameIn, &language))
    {
        PyErr_SetString(DetectSQLEncodingError, "Parsing arguments failed. Check the argument list");
        return NULL;
    }
    fileIn = fopen(filenameIn, "rb");
    if (!fileIn)
    {
        PyErr_SetString(DetectSQLEncodingError, "Loading file failed");
        return NULL;
    }
    encoding = detectSingleByteKnownLanguage(fileIn, (size_t)language);
    fclose(fileIn);
    return PyBytes_FromString(encoding);
}
/*
 Function:  detectsqlencoding_detectEnca
 --------------------
 wrapper for detectEnca
 IMPORTANT: better use detectsqlencoding_detectSingleByteKnownLanguage to get the possibility to use all supported languages
 filenameIn (in args): name of file to be investigated
 language (in args): known language

 returns: detected encoding using enca, "unknown" otherwise
 */
static PyObject* detectsqlencoding_detectEnca(PyObject* self, PyObject* args)
{
    const char* filenameIn;
    const char* encoding;
    FILE* fileIn;
    int language;
    if (!PyArg_ParseTuple(args, "si", &filenameIn, &language))
    {
        PyErr_SetString(DetectSQLEncodingError, "Parsing arguments failed. Check the argument list");
        return NULL;
    }
    fileIn = fopen(filenameIn, "rb");
    if (!fileIn)
    {
        PyErr_SetString(DetectSQLEncodingError, "Loading file failed");
        return NULL;
    }
    encoding = detectEnca(fileIn, (size_t)language);
    fclose(fileIn);
    return PyBytes_FromString(encoding);
}
/*
 Function:  detectsqlencoding_detectFarEastSafeMultibyte
 --------------------
 wrapper for detectFarEastSafeMultibyte
 filenameIn (in args): name of file to be investigated
 tolerance (in args): tolerance for detection (>=0 and <1)

 returns: detected far east safe multibyte encoding, "unknown" otherwise
 */
static PyObject* detectsqlencoding_detectFarEastSafeMultibyte(PyObject* self, PyObject* args)
{
    char* filenameIn;
    float tolerance;
    FILE* fileIn;
    if (!PyArg_ParseTuple(args, "sf", &filenameIn, &tolerance))
    {
        PyErr_SetString(DetectSQLEncodingError, "Parsing arguments failed. Check the argument list");
        return NULL;
    }
    fileIn = fopen(filenameIn, "rb");
    if (!fileIn)
    {
        PyErr_SetString(DetectSQLEncodingError, "Loading file failed");
        return NULL;
    }
    const char* encoding = detectFarEastSafeMultibyte(fileIn, tolerance);
    fclose(fileIn);
    return PyBytes_FromString(encoding);
}
/*
 Function:  detectsqlencoding_isUTF8
 --------------------
 wrapper for isUTF8
 filenameIn (in args): name of file to be investigated
 tolerance (in args): tolerance for detection (>=0 and <1)

 returns: true if it is UTF-8, false otherwise
 */
static PyObject* detectsqlencoding_isUTF8(PyObject* self, PyObject* args)
{
    char* filenameIn;
    float tolerance;
    FILE* fileIn;
    if (!PyArg_ParseTuple(args, "sf", &filenameIn, &tolerance)) //read arguments to cstring and float
    {
        PyErr_SetString(DetectSQLEncodingError, "Parsing arguments failed. Check the argument list");
        return NULL; //error
    }
    fileIn = fopen(filenameIn, "rb"); //read binary
    if (!fileIn)
    {
        PyErr_SetString(DetectSQLEncodingError, "Loading file failed");
        return NULL; //error
    }
    _Bool result = isUTF8(fileIn, tolerance); //c-function call
    
    return PyBool_FromLong(result); //return Python-Bool
}
/*
 Function:  detectsqlencoding_detectEncodingOtherLanguages
 --------------------
 wrapper for detectEncodingOtherLanguages
 filenameIn (in args): name of file to be investigated
 language (in args): known language

 returns: detected encoding using language dependent algorithm
 */
static PyObject* detectsqlencoding_detectEncodingOtherLanguages(PyObject* self, PyObject* args)
{
    const char* filenameIn;
    FILE* fileIn;
    int language;
    if (!PyArg_ParseTuple(args, "si", &filenameIn, &language))
    {
        PyErr_SetString(DetectSQLEncodingError, "Parsing arguments failed. Check the argument list");
        return NULL;
    }
    fileIn = fopen(filenameIn, "rb");
    if (!fileIn)
    {
        PyErr_SetString(DetectSQLEncodingError, "Loading file failed");
        return NULL;
    }
    const char* encoding = detectEncodingOtherLanguages(fileIn, (size_t)language);
    fclose(fileIn);
    return PyBytes_FromString(encoding);
}
/*
 Function:  detectsqlencoding_detectEncodingUnknownLanguage
 --------------------
 wrapper for detectEncodingUnknownLanguage
 filenameIn (in args): name of file to be investigated

 returns: detected encoding using language independent algorithm
 */
static PyObject* detectsqlencoding_detectEncodingUnknownLanguage(PyObject* self, PyObject* args)
{
    char* filenameIn;
    FILE* fileIn;
    if (!PyArg_ParseTuple(args, "s", &filenameIn))
    {
        PyErr_SetString(DetectSQLEncodingError, "Parsing arguments failed. Check the argument list");
        return NULL;
    }
    fileIn = fopen(filenameIn, "rb");
    if (!fileIn)
    {
        PyErr_SetString(DetectSQLEncodingError, "Loading file failed");
        return NULL;
    }
    const char* encoding = detectEncodingUnknownLanguage(fileIn);
    fclose(fileIn);
    return PyBytes_FromString(encoding);
}
/*
 Function:  detectsqlencoding_highlightNonASCII
 --------------------
 wrapper for initDetectSQLEncoding
 bufferIn (in args): string to be highlighted

 returns: highlighted string
 */
static PyObject* detectsqlencoding_highlightNonASCII(PyObject* self, PyObject* args)
{
    Py_buffer bufferIn;
    if (!PyArg_ParseTuple(args, "s*", &bufferIn))
    {
        PyErr_SetString(DetectSQLEncodingError, "Parsing arguments failed. Check the argument list");
        return NULL;
    }
    const char* highlightedText = highlightNonASCII((char*)bufferIn.buf);
    
    return PyBytes_FromString(highlightedText);
}
/*
 Function:  detectsqlencoding_getCountLanguages
 --------------------
 wrapper for getCountLanguages

 returns: count of supported languages
 */
static PyObject* detectsqlencoding_getCountLanguages(PyObject* self, PyObject* args)
{
    if (!PyArg_ParseTuple(args, ""))
    {
        PyErr_SetString(DetectSQLEncodingError, "Parsing arguments failed. Check the argument list");
        return NULL;
    }
    return PyLong_FromLong(getCountLanguages());
}
/*
 Function:  detectsqlencoding_getLanguages
 --------------------
 wrapper for getLanguages

 returns: tuple of supported languages
 */
static PyObject* detectsqlencoding_getLanguages(PyObject* self, PyObject* args)
{
    if (!PyArg_ParseTuple(args, ""))
    {
        PyErr_SetString(DetectSQLEncodingError, "Parsing arguments failed. Check the argument list");
        return NULL;
    }
    PyObject* tupleLanguages = PyTuple_New(getCountLanguages());
    for (size_t i = 0; i < getCountLanguages(); i++)
    {
        PyTuple_SetItem(tupleLanguages, i, PyBytes_FromString(getLanguages()[i]));
    }
    return tupleLanguages;
}
/*
 Function:  detectsqlencoding_encodingIsFarEastSafeMultibyte
 --------------------
 wrapper for encodingIsFarEastSafeMultibyte
 encoding (in args): name of encoding to be investigated

 returns: true if it is far east save multibyte, false otherwise
 */
static PyObject* detectsqlencoding_encodingIsFarEastSafeMultibyte(PyObject* self, PyObject* args)
{
    char* encoding;
    if (!PyArg_ParseTuple(args, "s", &encoding)) //read arguments to cstring
    {
        PyErr_SetString(DetectSQLEncodingError, "Parsing arguments failed. Check the argument list");
        return NULL; //error
    }
    _Bool result = encodingIsFarEastSafeMultibyte(encoding); //c-function call
    return PyBool_FromLong(result); //return Python-Bool
}

static PyMethodDef detectsqlencodingMethods[] = {
     {"init",  detectsqlencoding_init, METH_VARARGS,"Initialize the encoding detector\nCalls the initializers for cleaning and detecting to initialize variables \n\nParameters: none\n\nReturns: None, in case of error another value"},
     {"clean",  detectsqlencoding_clean, METH_VARARGS,"Clean SQL-file to plaintext\nCleanes a given SQL-file by removing parts which may cause negative effects for detection.\n\nParameters: \nfileIn: File to read (input)\nfileCleaned: output-file (pure text), containing the pure content after execution\n\nReturns: None, in case of error another value"},
     {"detectSingleByteKnownLanguage",  detectsqlencoding_detectSingleByteKnownLanguage, METH_VARARGS,"Detects the encoding of a file with a known language\n\nParameters:\nfileIn: file to be investigated\nlanguageNumber: number of language from getLanguages()\n\nReturns: String containing encoding guessed by enca or having a look at the language in general. If detection fails: \"unknown\"\n"},
     {"detectFarEastSafeMultibyte",  detectsqlencoding_detectFarEastSafeMultibyte, METH_VARARGS,"Guesses the encoding of a file if it es a safe detectable encoding from far east with a tolerance\n\nParameters:\nfileIn: file to be investigated\ntolerance: value of tolerance for the detections. Should be >=0 and <1\n\nReturns: String containing encoding if one of the far-east-safe-multibyte-encodings is detected (considering the tolerance).If not: \"unknown\""},
     {"detectEncodingOtherLanguages",  detectsqlencoding_detectEncodingOtherLanguages, METH_VARARGS,"Guesses the encoding of a file which is written in a language contained in getLanguages()\n\nParameters:\nfileIn: file to be investigated\nlanguageNumber: number of language in languages\n\nReturns: String containing encoding detected usingthe given language (fallback option: ICU)"},
     {"detectEncodingUnknownLanguage",  detectsqlencoding_detectEncodingUnknownLanguage, METH_VARARGS,"Guesses the encoding of a file it is an unknown language\n\nParameters:\nfileIn: file to be investigated\n\nReturns: String containing encoding detected without knowing a language (fallback option: ICU)"},
     {"highlightNonASCII",  detectsqlencoding_highlightNonASCII, METH_VARARGS,"Highlight Non-ASCII-Characters in a given string\n\nParameters:\ninput: text to be highlighted\n\nReturns: Highlighted text as const char*"},
     {"isUTF8",  detectsqlencoding_isUTF8, METH_VARARGS,"Detects if the file is encoded using UTF-8\n\nParameters:\nfileIn: file to be investigated\ntolerance: value of tolerance for UTF-8 detection. Should be >=0 and <1\n\nReturns: True, if the file is encoded in UTF-8 considering the tolerance. Otherwise: false"},
     {"getCountLanguages",  detectsqlencoding_getCountLanguages, METH_VARARGS,"Get the count of the directly supported languages\n\nParameters: none\n\nReturns: Number of all initialized languages including enca before using getCountLanguages, initDetectTextEncoding is requiered. Otherwise Returns 0"},
     {"getLanguages",  detectsqlencoding_getLanguages, METH_VARARGS,"Get the directly supported languages as a tuple\n\nParameters: none\n\nReturns: All initialized languages including enca before using getLanguages, initDetectTextEncoding is requiered. Otherwise Returns NULL"},
     {"encodingIsFarEastSafeMultibyte",  detectsqlencoding_encodingIsFarEastSafeMultibyte, METH_VARARGS,"Checks if the given encoding is safe detectable Far East Multibyte\n\nParameters:\nencoding: string to be checked\n\nReturns: True, if a given string is equal to a far-east-safe-multibyte-encoding"},
     {"detectEnca",  detectsqlencoding_detectEnca, METH_VARARGS,"Detects using pure enca\n\nParameters:\nfileIn: file to be investigated\nlanguageNumber: number of language in languages\n\nReturns: String containing encoding detected by enca using the given language. If detection fails: \"unknown\""},
     {NULL, NULL, 0, NULL}
};
static struct PyModuleDef detectsqlencodingmodule = {
    PyModuleDef_HEAD_INIT,
    "detectsqlencoding",   /* name of module */
    DETECTSQLENCODING_DOC, /* module documentation, may be NULL */
    -1,       /* size of per-interpreter state of the module,
                 or -1 if the module keeps state in global variables. */
    detectsqlencodingMethods
};
/*
 Function:  PyInit_detectsqlencoding
 --------------------
 initialises the module creation

 returns: return value of PyModule_Create
 */
PyMODINIT_FUNC PyInit_detectsqlencoding(void)
{
    PyObject* m;
    m = PyModule_Create(&detectsqlencodingmodule);
    if (m == NULL)
        return NULL;
    DetectSQLEncodingError = PyErr_NewException("detectsqlencoding.error", NULL, NULL); //new Exception-type for this module
    Py_XINCREF(DetectSQLEncodingError);
    if (PyModule_AddObject(m, "error", DetectSQLEncodingError) < 0) {
        Py_XDECREF(DetectSQLEncodingError);
        Py_CLEAR(DetectSQLEncodingError);
        Py_DECREF(m);
        return NULL;
    }
    return m;
}
/*
 Function:  main
 --------------------
 builds a Python-library

 returns: 0 when terminates without errors, 1 in case of errors
 */
int main(int argc, char* argv[])
{
    wchar_t* program = Py_DecodeLocale(argv[0], NULL);
    if (program == NULL) {
        fprintf(stderr, "Fatal error: cannot decode argv[0]\n");
        exit(1);
    }

    /* Add a built-in module, before Py_Initialize */
    if (PyImport_AppendInittab("detectsqlencoding", PyInit_detectsqlencoding) == -1) {
        fprintf(stderr, "Error: could not extend in-built modules table\n");
        exit(1);
    }

    /* Pass argv[0] to the Python interpreter */
    Py_SetProgramName(program);

    /* Initialize the Python interpreter.  Required.
       If this step fails, it will be a fatal error. */
    Py_Initialize();

    /* Optionally import the module; alternatively,
       import can be deferred until the embedded script
       imports it. */
    PyObject* pmodule = PyImport_ImportModule("detectsqlencoding");
    if (!pmodule) {
        PyErr_Print();
        fprintf(stderr, "Error: could not import module 'detectsqlencoding'\n");
    }

    PyMem_RawFree(program);
    return 0;
}
