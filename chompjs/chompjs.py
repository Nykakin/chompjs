# -*- coding: utf-8 -*-

import json
import sys

from _chompjs import parse

def parse_js_object(string, initial_stack_size=10, unicode_escape=False):
    if unicode_escape:
        string = string.encode().decode('unicode_escape')
    if sys.version_info[0] < 3:
        string = string.encode('utf-8')
    return json.loads(parse(string, initial_stack_size))
