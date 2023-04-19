----- Linux -----
[python3 installation needed]
-- build and install module (.so) --
    make clean
    make
    make install

-> detectfileencoding.so will be generated

-- build wheel (.whl) --
[pip installation needed]

    make clean
    make wheel

-> .whl file is in "dist". Do not rename!

-- install wheel --
[in directory of wheel-file]
    pip install detectsqlencoding-1.0-[...]linux[...].whl
[attention: use the correct file for linux]




