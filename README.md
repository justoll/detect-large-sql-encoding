# detect-large-sql-encoding
Detects or guesses the encoding of large given SQL-files (different SQL-formats).

The library is especially designed for (very) large SQL files. Many GB are not a problem and in many cases this also takes only a few (milli-)seconds. Nevertheless, for some encodings a statistical analysis is required, this causes a longer computation time in these cases for good results.

This Library is also **resilient to isolated encoding errors** or small areas of deviating encoding. 




----- Linux  -----

You need the tool **"enca"** installed from https://github.com/nijel/enca. Install it with

    ./configure
    make 
    make check
    make install

The **ICU-libs** in ./icu/lib have to be copied to /usr/local/lib.

Then continue with the Readme.md-files in the subdirectories. You will need libDetectSQLEncoding as a basis for the standalone and Python version
