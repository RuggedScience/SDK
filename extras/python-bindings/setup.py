# Always prefer setuptools over distutils
from setuptools import setup, Extension

from Cython.Build import cythonize


rsdio_extension = Extension(
    'rssdk.rsdio',
    language='c++',
    sources=['src/rssdk/rsdio.pyx'],
    libraries=['rsdio'],
    library_dirs=["C:/Users/rugged/Documents/GitHub/SDK/build/dio/Release"],
    include_dirs=["C:/Users/rugged/Documents/GitHub/SDK/utils", "C:/Users/rugged/Documents/GitHub/SDK/dio/include", "C:/Users/rugged/Documents/GitHub/SDK/build/dio/exports"]
)

rspoe_extension = Extension(
    'rssdk.rspoe',
    language='c++',
    sources=['src/rssdk/rspoe.pyx'],
    libraries=['rspoe'],
    library_dirs=["C:/Users/rugged/Documents/GitHub/SDK/build/poe/Release"],
    include_dirs=["C:/Users/rugged/Documents/GitHub/SDK/utils", "C:/Users/rugged/Documents/GitHub/SDK/poe/include", "C:/Users/rugged/Documents/GitHub/SDK/build/poe/exports"]
)


setup(
    package_dir={"": "src"},
    packages=['rssdk'],
    ext_modules=cythonize(
        [rsdio_extension, rspoe_extension],
        language_level=3.6,
        compiler_directives={"linetrace": True} # Opt-in via CYTHON_TRACE macro
    ),
    setup_requires=[
        'cython >= 0.22.1',
    ],
    zip_safe=False,
)
