from setuptools import setup, Extension

module1 = Extension('detectsqlencoding',
                    define_macros = [('MAJOR_VERSION', '1'),
                                     ('MINOR_VERSION', '0')],
                    include_dirs = ['../libDetectSQLEncoding/include/', '../icu/include/', '../enca/lib/'],
                    libraries = ['libDetectSQLEncoding', 'libEnca', 'icudt', 'icuin', 'icuio', 'icutest', 'icutu', 'icuuc'],
                    library_dirs = ['../libDetectSQLEncoding/lib/x64/Release/', '../enca/x64/Release/', '../icu46/lib64/'],
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


