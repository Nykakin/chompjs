# -*- coding: utf-8 -*-

import json

from _chompjs import parse, parse_objects


def _preprocess(string, unicode_escape=False):
    if not string:
        raise ValueError('Invalid input')
    if unicode_escape:
        string = string.encode().decode('unicode_escape')
    return string


def parse_js_object(string, unicode_escape=False, jsonlines=False, json_params=None):
    string = _preprocess(string, unicode_escape)
    if not json_params:
        json_params = {}

    parsed_data = parse(string, jsonlines)

    if jsonlines:
        return [json.loads(j, **json_params) for j in parsed_data.split('\0')]
    else:
        return json.loads(parsed_data, **json_params)


def parse_js_objects(string, unicode_escape=False, omitempty=False, json_params=None):
    string = _preprocess(string, unicode_escape)
    if not json_params:
        json_params = {}
    for raw_data in parse_objects(string):
        try:
            data = json.loads(raw_data, **json_params)
        except ValueError:
            continue

        if data:
            yield data
