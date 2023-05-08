# -*- coding: utf-8 -*-

import json

from _chompjs import parse, parse_objects


def _preprocess(string, unicode_escape=False):
    if unicode_escape:
        string = string.encode().decode('unicode_escape')
    return string


def parse_js_object(string, unicode_escape=False, json_params=None):
    """
    Extracts first JSON object encountered in the input string

    Parameters
    ----------
    string: str
        Input string

    >>> parse_js_object("{a: 100}")
    {'a': 100}

    unicode_escape: bool, optional
        Attempt to fix input string if it contains escaped special characters

    json_params: dict, optional
        Allow passing down standard json.loads flags

    >>> parse_js_object("{'a': 10.1}")
    {'a': 10.1}
    >>> import decimal
    >>> parse_js_object("{'a': 10.1}", json_params={'parse_float': decimal.Decimal})
    {'a': Decimal('10.1')}

    Returns
    -------
    list | dict
        Extracted JSON object

    Raises
    ------
    ValueError
        If failed to parse input properly

    ```python
    >>> parse_js_object(None)
    Traceback (most recent call last):
      File "<stdin>", line 1, in <module>
      File "/home/mariusz/Documents/Projekty/chompjs/chompjs/chompjs.py", line 49, in parse_js_object
        raise ValueError('Invalid input')
    ValueError: Invalid input
    >>> parse_js_object("")
    Traceback (most recent call last):
      File "<stdin>", line 1, in <module>
      File "/home/mariusz/Documents/Projekty/chompjs/chompjs/chompjs.py", line 49, in parse_js_object
        raise ValueError('Invalid input')
    ValueError: Invalid input
    >>> parse_js_object("{'a': 12")
    Traceback (most recent call last):
      File "<stdin>", line 1, in <module>
      File "/home/mariusz/Documents/Projekty/chompjs/chompjs/chompjs.py", line 55, in parse_js_object
        parsed_data = parse(string)
    ValueError: Error parsing input near character 15
    ```
    """
    if not string:
        raise ValueError('Invalid input')

    string = _preprocess(string, unicode_escape)
    if not json_params:
        json_params = {}

    parsed_data = parse(string)
    return json.loads(parsed_data, **json_params)


def parse_js_objects(string, unicode_escape=False, omitempty=False, json_params=None):
    """
    Returns a generator extracting all JSON objects encountered in the input string

    Parameters
    ----------
    string: str
        Input string

    >>> it = parse_js_objects("{a: 100} {b: 100}")
    >>> next(it)
    {'a': 100}
    >>> next(it)
    {'b': 100}

    unicode_escape: bool, optional
        Attempt to fix input string if it contains escaped special characters

    omitempty: bool, optional
        Skip empty dictionaries and lists

    >>> list(parse_js_objects("{a: 12} {} {b: 13}"))
    [{'a': 12}, {}, {'b': 13}]
    >>> list(parse_js_objects("{a: 12} {} {b: 13}", omitempty=True))
    [{'a': 12}, {'b': 13}]

    json_params: dict, optional
        Allow passing down standard json.loads flags

    >>> next(parse_js_objects("{'a': 10.1}"))
    {'a': 10.1}
    >>> import decimal
    >>> next(parse_js_objects("{'a': 10.1}", json_params={'parse_float': decimal.Decimal}))
    {'a': Decimal('10.1')}

    Returns
    -------
    generator
        Iterating over it yields all encountered JSON objects
    """

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
