# Always prefer setuptools over distutils
from setuptools import setup, Extension

from Cython.Build import cythonize


rsdio_extension = Extension(
    'rssdk.rsdio',
    language='c++',
    sources=['src/rssdk/rsdio.pyx'],
    libraries=['rsdio'],
)

rspoe_extension = Extension(
    'rssdk.rspoe',
    language='c++',
    sources=['src/rssdk/rspoe.pyx'],
    libraries=['rspoe'],
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