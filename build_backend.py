import os
import sys

from setuptools.build_meta import *  # noqa: F403


def _bootstrap():
    if not sys.platform.startswith("linux"):
        return

    for variable in ("CFLAGS", "LDFLAGS"):
        value = os.environ.get(variable, "")
        if "-Wl,-Bsymbolic-functions" not in value:
            os.environ[variable] = f"{value} -Wl,-Bsymbolic-functions".strip()


_bootstrap()
