import json

from hello import parse

def parse_js_object(string):
    return json.loads(parse(string))
