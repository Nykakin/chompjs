#!/usr/bin/env python3
# encoding: utf-8

from os import path
from setuptools import setup, Extension


this_directory = path.abspath(path.dirname(__file__))
with open(path.join(this_directory, 'README.md'), encoding='utf-8') as f:
    long_description = f.read()


chompjs_extension = Extension('_chompjs', sources=['module.c', 'parser.c'])

setup(
    name='chompjs',
    version='1.0.1',
    description='Parsing JavaScript objects into Python dictionaries',
    author='Mariusz Obajtek',
    author_email='nykakin@gmail.com',
    keywords='parsing parser JavaScript json',
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
    ],
    python_requires='>=3.6',
    url='https://github.com/Nykakin/chompjs',
    long_description=long_description,
    long_description_content_type='text/markdown',
    include_package_data=True,
    packages=['chompjs'],
)
