----- Linux  -----
[The ICU-libs in ./icu/lib have to be copied to /usr/local/lib and enca to be installed (see other README.md)]

    sudo ldconfig
    [add line] /usr/local/lib

    make clean
    make 
    make install

-> installs .so file to /usr/local/lib
