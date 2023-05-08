# Chompjs

![license](https://img.shields.io/github/license/Nykakin/chompjs?style=flat-square)
![pypi version](https://img.shields.io/pypi/v/chompjs.svg)
![python version](https://img.shields.io/pypi/pyversions/chompjs.svg)
![downloads](https://img.shields.io/pypi/dm/chompjs.svg)

Transforms JavaScript objects into Python dictionaries.

In web scraping, you sometimes need to transform Javascript objects embedded in HTML pages into valid Python dictionaries. `chompjs` is a library designed to be a more powerful replacement of standard `json.loads`.

```python
>>> chompjs.parse_js_object("{a: 100}")
{'a': 100}
>>>
>>> json_lines = """
... {'a': 12}
... {'b': 13}
... {'c': 14}
... """
>>> for entry in chompjs.parse_js_objects(json_lines):
...     print(entry)
... 
{'a': 12}
{'b': 13}
{'c': 14}
```

[Reference documentation](https://nykakin.github.io/chompjs/)

## Quickstart

**1. installation**

```
> pip install chompjs
```

or build from source:

```bash
$ git clone https://github.com/Nykakin/chompjs
$ cd chompjs
$ python setup.py build
$ python setup.py install
```

## Features

There are two functions available:
* `parse_js_object` - try reading first encountered JSON-like object. Raises `ValueError` on failure
* `parse_js_objects` - returns a generator yielding all encountered JSON-like objects. Can be used to read [JSON Lines](https://jsonlines.org/)

An example usage with `scrapy`:

```python
import chompjs
import scrapy


class MySpider(scrapy.Spider):
    # ...

    def parse(self, response):
        script_css = 'script:contains("__NEXT_DATA__")::text'
        script_pattern = r'__NEXT_DATA__ = (.*);'
        # warning: for some pages you need to pass replace_entities=True
        # into re_first to have JSON escaped properly
        script_text = response.css(script_css).re_first(script_pattern)
        try:
            json_data = chompjs.parse_js_object(script_text)
        except ValueError:
            self.log('Failed to extract data from {}'.format(response.url))
            return

        # work on json_data
```

Parsing of [JSON5 objects](https://json5.org/) is supported:

```python
>>> data = """
... {
...   // comments
...   unquoted: 'and you can quote me on that',
...   singleQuotes: 'I can use "double quotes" here',
...   lineBreaks: "Look, Mom! \
... No \\n's!",
...   hexadecimal: 0xdecaf,
...   leadingDecimalPoint: .8675309, andTrailing: 8675309.,
...   positiveSign: +1,
...   trailingComma: 'in objects', andIn: ['arrays',],
...   "backwardsCompatible": "with JSON",
... }
... """
>>> chompjs.parse_js_object(data)
{'unquoted': 'and you can quote me on that', 'singleQuotes': 'I can use "double quotes" here', 'lineBreaks': "Look, Mom! No \n's!", 'hexadecimal': 912559, 'leadingDecimalPoint': 0.8675309, 'andTrailing': 8675309.0, 'positiveSign': '+1', 'trailingComma': 'in objects', 'andIn': ['arrays'], 'backwardsCompatible': 'with JSON'}
```

If the input string is not yet escaped and contains a lot of `\\` characters, then `unicode_escape=True` argument might help to sanitize it:

```python
>>> chompjs.parse_js_object('{\\\"a\\\": 12}', unicode_escape=True)
{'a': 12}
```

By default `chompjs` tries to start with first `{` or `[` character it founds, omitting the rest:

```python
>>> chompjs.parse_js_object('<div>...</div><script>foo = [1, 2, 3];</script><div>...</div>')
[1, 2, 3]
```

`json_params` argument can be used to pass options to underlying `json_loads`, such as `strict` or `object_hook`:

```python
>>> import decimal
>>> import chompjs
>>> chompjs.parse_js_object('[23.2]', json_params={'parse_float': decimal.Decimal})
[Decimal('23.2')]
```

# Rationale

In web scraping data often is not present directly inside HTML, but instead provided as an embedded JavaScript object that is later used to initialize the page, for example:

```html
<html>
<head>...</head>
<body>
...
<script type="text/javascript">window.__PRELOADED_STATE__={"foo": "bar"}</script>
...
</body>
</html>
```

Standard library function `json.loads` is usually sufficient to extract this data:

```python
>>> # scrapy shell file:///tmp/test.html
>>> import json
>>> script_text = response.css('script:contains(__PRELOADED_STATE__)::text').re_first('__PRELOADED_STATE__=(.*)')
>>> json.loads(script_text)
{u'foo': u'bar'}

```
The problem is that not all valid JavaScript objects are also valid JSONs. For example all those strings are valid JavaScript objects but not valid JSONs:

* `"{'a': 'b'}"` is not a valid JSON because it uses `'` character to quote
* `'{a: "b"}'`is not a valid JSON because property name is not quoted at all
* `'{"a": [1, 2, 3,]}'` is not a valid JSON because there is an extra `,` character at the end of the array
* `'{"a": .99}'` is not a valid JSON because float value lacks a leading 0

As a result, `json.loads` fail to extract any of those:

```
>>> json.loads("{'a': 'b'}")
Traceback (most recent call last):
  File "<console>", line 1, in <module>
  File "/usr/lib/python3.10/json/__init__.py", line 339, in loads
    return _default_decoder.decode(s)
  File "/usr/lib/python3.10/json/decoder.py", line 364, in decode
    obj, end = self.raw_decode(s, idx=_w(s, 0).end())
  File "/usr/lib/python3.10/json/decoder.py", line 380, in raw_decode
    obj, end = self.scan_once(s, idx)
ValueError: Expecting property name: line 1 column 2 (char 1)
>>> json.loads('{a: "b"}')
Traceback (most recent call last):
  File "<console>", line 1, in <module>
  File "/usr/lib/python3.10/json/__init__.py", line 339, in loads
    return _default_decoder.decode(s)
  File "/usr/lib/python3.10/json/decoder.py", line 364, in decode
    obj, end = self.raw_decode(s, idx=_w(s, 0).end())
  File "/usr/lib/python3.10/json/decoder.py", line 380, in raw_decode
    obj, end = self.scan_once(s, idx)
ValueError: Expecting property name: line 1 column 2 (char 1)
>>> json.loads('{"a": [1, 2, 3,]}')
Traceback (most recent call last):
  File "<console>", line 1, in <module>
  File "/usr/lib/python3.10/json/__init__.py", line 339, in loads
    return _default_decoder.decode(s)
  File "/usr/lib/python3.10/json/decoder.py", line 364, in decode
    obj, end = self.raw_decode(s, idx=_w(s, 0).end())
  File "/usr/lib/python3.10/json/decoder.py", line 382, in raw_decode
    raise ValueError("No JSON object could be decoded")
ValueError: No JSON object could be decoded
>>> json.loads('{"a": .99}')
Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
  File "/usr/lib/python3.7/json/__init__.py", line 348, in loads
    return _default_decoder.decode(s)
  File "/usr/lib/python3.7/json/decoder.py", line 337, in decode
    obj, end = self.raw_decode(s, idx=_w(s, 0).end())
  File "/usr/lib/python3.7/json/decoder.py", line 355, in raw_decode
    raise JSONDecodeError("Expecting value", s, err.value) from None
json.decoder.JSONDecodeError: Expecting value: line 1 column 7 (char 6)

```
`chompjs` library was designed to bypass this limitation, and it allows to scrape such JavaScript objects into proper Python dictionaries:

```
>>> import chompjs
>>> 
>>> chompjs.parse_js_object("{'a': 'b'}")
{u'a': u'b'}
>>> chompjs.parse_js_object('{a: "b"}')
{u'a': u'b'}
>>> chompjs.parse_js_object('{"a": [1, 2, 3,]}')
{u'a': [1, 2, 3]}
```

Internally `chompjs` use a parser written in C to iterate over raw string, fixing its issues along the way. The final result is then passed down to standard library's `json.loads`, ensuring a high speed as compared to full-blown JavaScript parsers such as `demjson`.

```
>>> import json
>>> import _chompjs
>>> 
>>> _chompjs.parse('{a: 1}')
'{"a":1}'
>>> json.loads(_)
{u'a': 1}
>>> chompjs.parse_js_object('{"a": .99}')
{'a': 0.99}
```

# Development
Pull requests are welcome. 

To run unittests

```
$ tox
```
