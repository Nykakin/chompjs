import json

from _chompjs import parse

def parse_js_object(string, initial_stack_size=10, unicode_escape=False):
    if unicode_escape:
        string = string.encode().decode('unicode_escape')
    return json.loads(parse(string, initial_stack_size))
