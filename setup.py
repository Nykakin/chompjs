import sys

from setuptools import Extension, setup


extra_args = ["-Wl,-Bsymbolic-functions"] if sys.platform.startswith("linux") else []

setup(
    ext_modules=[
        Extension(
            "_chompjs",
            sources=[
                "_chompjs/module.c",
                "_chompjs/parser.c",
                "_chompjs/buffer.c",
            ],
            extra_compile_args=extra_args,
            extra_link_args=extra_args,
        )
    ]
)
