# -*- coding: utf-8 -*-
from __future__ import annotations

import json
import warnings
from typing import Any, Protocol, TypeVar, TYPE_CHECKING
from _chompjs import parse, parse_objects # type: ignore[reportAttributeAccessIssue,attr-defined]


if TYPE_CHECKING:
    from collections.abc import Iterable, Mapping, Sequence

    _T = TypeVar("_T")
    _T2 = TypeVar("_T2")

    class _JsonLoader(Protocol):

        def __call__(self, obj: str, / , *args: Any, **kwargs: Any) -> Any: ...


def _preprocess(string: str, unicode_escape: bool=False) -> str:
    if unicode_escape:
        string = string.encode().decode("unicode_escape")
    return string


def _process_loader_arguments(
    loader_args: Sequence[_T] | None, 
    loader_kwargs: Mapping[str, _T2] | None, 
    json_params: Mapping[str, _T2] | None,
) -> tuple[Sequence[_T], Mapping[str, _T2]]:
    if json_params:
        msg = "json_params argument is deprecated, please use loader_kwargs instead"
        warnings.warn(msg, DeprecationWarning)
        loader_kwargs = json_params

    if not loader_args:
        loader_args = []

    if not loader_kwargs:
        loader_kwargs = {}

    return (loader_args, loader_kwargs)


def parse_js_object(
    string: str,
    unicode_escape: bool=False,
    loader: _JsonLoader=json.loads,
    loader_args: Sequence[Any] | None=None,
    loader_kwargs: Mapping[str, Any] | None=None,
    json_params: Mapping[str, Any] | None=None,
) -> Any:
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

    >>> parse_js_object('{\\\\"a\\\\": 100}')
    {'\\\\"a\\\\"': 100}
    >>> parse_js_object('{\\\\"a\\\\": 100}', unicode_escape=True)
    {'a': 100}

    loader: func, optional
        Function used to load processed input data. By default `json.loads` is used

    >>> import orjson
    >>> import chompjs
    >>> 
    >>> chompjs.parse_js_object("{'a': 12}", loader=orjson.loads)
    {'a': 12}

    loader_args: list, optional
        Allow passing down positional arguments to loader function

    loader_kwargs: dict, optional
        Allow passing down keyword arguments to loader function

    >>> parse_js_object("{'a': 10.1}")
    {'a': 10.1}
    >>> import decimal
    >>> parse_js_object("{'a': 10.1}", loader_kwargs={'parse_float': decimal.Decimal})
    {'a': Decimal('10.1')}

    .. deprecated:: 1.3.0
    json_params: dict, optional
        Use `loader_kwargs` instead

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
      ...
    ValueError: Invalid input
    >>> parse_js_object("No JSON objects in sight...")
    Traceback (most recent call last):
      ...
    json.decoder.JSONDecodeError: Expecting value: line 1 column 1 (char 0)

    ```

    """
    if not string:
        raise ValueError("Invalid input")

    loader_args, loader_kwargs = _process_loader_arguments(
        loader_args, loader_kwargs, json_params
    )

    string = _preprocess(string, unicode_escape)
    parsed_data = parse(string)
    return loader(parsed_data, *loader_args, **loader_kwargs)


def parse_js_objects(
    string: str,
    unicode_escape: bool=False,
    omitempty: bool=False, 
    loader: _JsonLoader=json.loads,
    loader_args: Sequence[Any] | None=None,
    loader_kwargs: Mapping[str, Any] | None=None,
    json_params: Mapping[str, Any] | None=None,
)-> Iterable[Any]:
    """
    Returns a generator extracting all JSON objects encountered in the input string.
    Can be used to read JSON Lines

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

    >>> next(parse_js_objects('{\\\\"a\\\\": 100}'))
    {'\\\\"a\\\\"': 100}
    >>> next(parse_js_objects('{\\\\"a\\\\": 100}', unicode_escape=True))
    {'a': 100}

    omitempty: bool, optional
        Skip empty dictionaries and lists

    >>> list(parse_js_objects("{a: 12} {} {b: 13}"))
    [{'a': 12}, {}, {'b': 13}]
    >>> list(parse_js_objects("{a: 12} {} {b: 13}", omitempty=True))
    [{'a': 12}, {'b': 13}]

    loader: func, optional
        Function used to load processed input data. By default `json.loads` is used

    >>> import orjson
    >>> import chompjs
    >>> 
    >>> next(chompjs.parse_js_objects("{'a': 12}", loader=orjson.loads))
    {'a': 12}

    loader_args: list, optional
        Allow passing down positional arguments to loader function

    loader_kwargs: dict, optional
        Allow passing down keyword arguments to loader function

    >>> next(parse_js_objects("{'a': 10.1}"))
    {'a': 10.1}
    >>> import decimal
    >>> next(parse_js_objects("{'a': 10.1}", loader_kwargs={'parse_float': decimal.Decimal}))
    {'a': Decimal('10.1')}

    .. deprecated:: 1.3.0
    json_params: dict, optional
        Use `loader_kwargs` instead

    Returns
    -------
    generator
        Iterating over it yields all encountered JSON objects
    """

    if not string:
        return

    loader_args, loader_kwargs = _process_loader_arguments(
        loader_args, loader_kwargs, json_params
    )

    string = _preprocess(string, unicode_escape)
    for raw_data in parse_objects(string):
        try:
            data = loader(raw_data, *loader_args, **loader_kwargs)
        except ValueError:
            continue

        if not data and omitempty:
            continue

        yield data
