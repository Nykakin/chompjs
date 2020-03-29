# -*- coding: utf-8 -*-

import json
import sys

from _chompjs import parse


def parse_js_object(string, initial_stack_size=10, unicode_escape=False):
    if not string:
        raise ValueError('Invalid input')
    if unicode_escape:
        string = string.encode().decode('unicode_escape')
    if sys.version_info[0] < 3:
        string = string.encode('utf-8')

    # I use this roundabout way to capture exception because Python 2.7 doesn't
    # support `raise ... from None` syntax and I don't want to include six.rethrow
    # only to change exception message
    exception = None
    try:
        parsed_json = parse(string, initial_stack_size)
    except ValueError as e:
        exception = e
    if exception:
        if sys.version_info[0] < 3:
            raise ValueError('Parser error: ... {}'.format(repr(str(exception))[1:-1]))
        else:
            raise ValueError("Parser error: ... {}".format(str(exception).encode('utf-8')))

    return json.loads(parsed_json)
