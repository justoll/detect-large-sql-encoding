----- Linux  -----

[first, create and install libDetectSQLEncoding. libDetectSQLEncoding.so has to be in this directory or installed to system. The ICU-libs in ./icu/lib have to be copied to /usr/local/lib]

    ldconfig
    make clean
    make 

-> creates binary detectSQLEncoding

