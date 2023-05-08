# -*- coding: utf-8 -*-

import json

from _chompjs import parse, parse_objects


def _preprocess(string, unicode_escape=False):
    if unicode_escape:
        string = string.encode().decode('unicode_escape')
    return string


def parse_js_object(string, unicode_escape=False, jsonlines=False, json_params=None):
    """Extracts first JSON object it encounters in the input string

    Parameters
    ----------
    string: str
        Input string
        >>> parse_js_object("{a: 100}")
        {'a': 100}

    unicode_escape: bool, optional
        Attempt to fix input string if it contains escaped special characters
        >>> parse_js_object('{\\"a\\": \\"b\\"}')
        {'\\"a\\"': '\\"b\\"'}
        >>> parse_js_object('{\\"a\\": \\"b\\"}', unicode_escape=True)
        {'a': 'b'}

    jsonlines: bool, optional
        Allow parsing jsonlines objects

        Raises
        ------
        ValueError
            If failed to parse input properly
    """


#        """Prints what the animals name is and what sound it makes.
#
#        If the argument `sound` isn't passed in, the default Animal
#        sound is used.
#
#        Parameters
#        ----------
#        sound : str, optional
#            The sound the animal makes (default is None)
#
#        Raises
#        ------
#        NotImplementedError
#            If no sound is set for the animal or passed in as a
#            parameter.
#        """


    if not string:
        raise ValueError('Invalid input')

    string = _preprocess(string, unicode_escape)
    if not json_params:
        json_params = {}

    parsed_data = parse(string, jsonlines)

    if jsonlines:
        return [json.loads(j, **json_params) for j in parsed_data.split('\0')]
    else:
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
