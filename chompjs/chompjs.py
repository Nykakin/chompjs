# -*- coding: utf-8 -*-

import json

from _chompjs import parse, parse_objects


def _preprocess(string, unicode_escape=False):
    if unicode_escape:
        string = string.encode().decode('unicode_escape')
    return string


def parse_js_object(string, unicode_escape=False, json_params=None):
    if not string:
        raise ValueError('Invalid input')

    string = _preprocess(string, unicode_escape)
    if not json_params:
        json_params = {}

    parsed_data = parse(string)
    return json.loads(parsed_data, **json_params)


def parse_js_objects(string, unicode_escape=False, omitempty=False, json_params=None):
    if not string:
        return

    string = _preprocess(string, unicode_escape)
    if not json_params:
        json_params = {}
    for raw_data in parse_objects(string):
        try:
            data = json.loads(raw_data, **json_params)
        except ValueError:
            continue
        
        if not data and omitempty:
            continue

        yield data
