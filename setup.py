#!/usr/bin/env python3
# encoding: utf-8

from io import open
from os import path
from platform import system
from setuptools import setup, Extension


this_directory = path.abspath(path.dirname(__file__))
with open(path.join(this_directory, 'README.md'), encoding='utf-8') as f:
    long_description = f.read()

extra_compile_args = []
extra_link_args = []
if system() == 'Linux':
    extra_compile_args = ['-Wl,-Bsymbolic-functions']
    extra_link_args = ['-Wl,-Bsymbolic-functions']

chompjs_extension = Extension(
    '_chompjs',
    sources=['_chompjs/module.c', '_chompjs/parser.c', '_chompjs/buffer.c'],
    extra_compile_args=extra_compile_args,
    extra_link_args=extra_link_args,
)

setup(
    name='chompjs',
    version='1.3.1',
    description='Parsing JavaScript objects into Python dictionaries',
    author='Mariusz Obajtek',
    author_email='nykakin@gmail.com',
    keywords='parsing parser JavaScript json json5 webscrapping',
    python_requires='>3.8',
    ext_modules=[chompjs_extension],
    classifiers=[
        "Programming Language :: Python :: 3",
        "Programming Language :: JavaScript",
        "Intended Audience :: Developers",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
        "Topic :: Software Development :: Libraries :: Python Modules",
        "Topic :: Text Processing :: General",
        "Topic :: Text Processing :: Linguistic",
        "Development Status :: 5 - Production/Stable",
        "Environment :: Console",
        "Environment :: Web Environment",
    ],
    url='https://github.com/Nykakin/chompjs',
    long_description=long_description,
    long_description_content_type='text/markdown',
    include_package_data=True,
    packages=['chompjs'],
)
