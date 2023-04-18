from setuptools import setup, Extension

module1 = Extension('detectsqlencoding',
                    define_macros = [('MAJOR_VERSION', '1'),
                                     ('MINOR_VERSION', '0')],
                    include_dirs = ['../libDetectSQLEncoding/include/', '../icu/include/', '../enca/lib/'],
                    libraries = [':libDetectSQLEncoding.so', ':libenca.so', ':libicudata.so', ':libicui18n.so', ':libicuio.so', ':libicutest.so', ':libicutu.so', ':libicuuc.so'],
                    library_dirs = ['../enca/lib/.libs', '../icu/lib', '../libDetectSQLEncoding/lib'],
                    sources = ['pyDetectSQLEncoding.c'])

setup (name = 'detectsqlencoding',
       version = '1.0',
       description = 'This package can be used to clean SQL-files to text files and detect the encoding of text files',
       author = 'Julian Stoll',
       author_email = '',
       url = '',
       long_description = '''
This package can be used to clean SQL-files to text files and detect the encoding of text files.
Before detecting use the init() function.
''',
       ext_modules = [module1])


