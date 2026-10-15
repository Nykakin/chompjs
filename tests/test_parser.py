from __future__ import unicode_literals

import math

import pytest

from chompjs import parse_js_object, parse_js_objects


@pytest.mark.parametrize(
    "in_data, expected_data",
    [
        ("{'hello': 'world'}", {"hello": "world"}),
        ("{'hello': 'world', 'my': 'master'}", {"hello": "world", "my": "master"}),
        (
            "{'hello': 'world', 'my': {'master': 'of Orion'}, 'test': 'xx'}",
            {"hello": "world", "my": {"master": "of Orion"}, "test": "xx"},
        ),
        ("{}", {}),
    ],
)
def test_parse_object(in_data, expected_data):
    result = parse_js_object(in_data)
    assert result == expected_data


@pytest.mark.parametrize(
    "in_data, expected_data",
    [
        ("[]", []),
        ("[[[]]]", [[[]]]),
        ("[[[1]]]", [[[1]]]),
        ("[1]", [1]),
        ("[1, 2, 3, 4]", [1, 2, 3, 4]),
        ("['h', 'e', 'l', 'l', 'o']", ["h", "e", "l", "l", "o"]),
        ("[[[[[[[[[[[[[[[1]]]]]]]]]]]]]]]", [[[[[[[[[[[[[[[1]]]]]]]]]]]]]]]),
    ],
)
def test_parse_list(in_data, expected_data):
    result = parse_js_object(in_data)
    assert result == expected_data


@pytest.mark.parametrize(
    "in_data, expected_data",
    [
        ("{'hello': [], 'world': [0]}", {"hello": [], "world": [0]}),
        ("{'hello': [1, 2, 3, 4]}", {"hello": [1, 2, 3, 4]}),
        ("[{'a':12}, {'b':33}]", [{"a": 12}, {"b": 33}]),
        (
            "[false, {'true': true, `pies`: \"kot\"}, false,]",
            [False, {"true": True, "pies": "kot"}, False],
        ),
        (
            "{a:1,b:1,c:1,d:1,e:1,f:1,g:1,h:1,i:1,j:1}",
            {k: 1 for k in "abcdefghij"},
        ),
        (
            "{'a':[{'b':1},{'c':[{'d':{'f':{'g':[1,2]}}},{'e':1}]}]}",
            {"a": [{"b": 1}, {"c": [{"d": {"f": {"g": [1, 2]}}}, {"e": 1}]}]},
        ),
    ],
)
def test_parse_mixed(in_data, expected_data):
    result = parse_js_object(in_data)
    assert result == expected_data


@pytest.mark.parametrize(
    "in_data, expected_data",
    [
        ("{'hello': 12, 'world': 10002.21}", {"hello": 12, "world": 10002.21}),
        ("[12, -323, 0.32, -32.22, .2, - 4]", [12, -323, 0.32, -32.22, 0.2, -4]),
        ('{"a": -12, "b": - 5}', {"a": -12, "b": -5}),
        ("{'a': true, 'b': false, 'c': null}", {"a": True, "b": False, "c": None}),
        ('["\\uD834\\uDD1E"]', ["𝄞"]),
        ("{'a': '123\\'456\\n'}", {"a": "123'456\n"}),
        ("['\u00e9']", ["é"]),
        ('{"cache":{"\u002ftest\u002f": 0}}', {"cache": {"/test/": 0}}),
        ('{"a": 3.125e7}', {"a": 3.125e7}),
        ("""{"a": "b\\'"}""", {"a": "b'"}),
        ('{"a": .99, "b": -.1}', {"a": 0.99, "b": -0.1}),
        ('["/* ... */", "// ..."]', ["/* ... */", "// ..."]),
        ('{"inclusions":["/*","/"]}', {"inclusions": ["/*", "/"]}),
    ],
)
def test_parse_standard_values(in_data, expected_data):
    result = parse_js_object(in_data)
    assert result == expected_data


def test_parse_nan():
    in_data = '{"A": NaN}'
    result = parse_js_object(in_data)
    assert math.isnan(result["A"])


@pytest.mark.parametrize(
    "in_data, expected_data",
    [
        ("{abc: 100, dev: 200}", {"abc": 100, "dev": 200}),
        ("{abcdefghijklmnopqrstuvwxyz: 12}", {"abcdefghijklmnopqrstuvwxyz": 12}),
        (
            "{age: function(yearBorn,thisYear) {return thisYear - yearBorn;}}",
            {"age": "function(yearBorn,thisYear) {return thisYear - yearBorn;}"},
        ),
        (
            "{\"abc\": function() {return '])))))))))))))))';}}",
            {"abc": "function() {return '])))))))))))))))';}"},
        ),
        ('{"a": undefined}', {"a": "undefined"}),
        ("[undefined, undefined]", ["undefined", "undefined"]),
        ("{_a: 1, $b: 2}", {"_a": 1, "$b": 2}),
        ("{regex: /a[^d]{1,12}/i}", {"regex": "/a[^d]{1,12}/i"}),
        ("{'a': function(){return '\"'}}", {"a": "function(){return '\"'}"}),
        ("{1: 1, 2: 2, 3: 3, 4: 4}", {"1": 1, "2": 2, "3": 3, "4": 4}),
        ("{'a': 121.}", {"a": 121.0}),
        ("{abc : 100}", {"abc": 100}),
        ("{abc     :       100}", {"abc": 100}),
        ("{abc: name }", {"abc": "name"}),
        ("{abc: name\t}", {"abc": "name"}),
        ("{abc: value\n}", {"abc": "value"}),
        ("{abc:  name}", {"abc": "name"}),
        ("{abc: \tname}", {"abc": "name"}),
        ("{abc: \nvalue}", {"abc": "value"}),
        (
            "{someProp: someArg => someFunc(someArg)}",
            {"someProp": "someArg => someFunc(someArg)"},
        ),
    ],
)
def test_parse_strange_values(in_data, expected_data):
    result = parse_js_object(in_data)
    assert result == expected_data


@pytest.mark.parametrize(
    "in_data, expected_data",
    [
        ('{"a": {"b": [12, 13, 14]}}text text', {"a": {"b": [12, 13, 14]}}),
        ('var test = {"a": {"b": [12, 13, 14]}}', {"a": {"b": [12, 13, 14]}}),
        ('{"a":\r\n10}', {"a": 10}),
        ("{'foo': 0,\r\n}", {"foo": 0}),
        (
            "{truefalse: 0, falsefalse: 1, nullnull: 2}",
            {"truefalse": 0, "falsefalse": 1, "nullnull": 2},
        ),
    ],
)
def test_strange_input(in_data, expected_data):
    result = parse_js_object(in_data)
    assert result == expected_data


@pytest.mark.parametrize(
    "in_data, expected_data",
    [
        ("[0]", [0]),
        ("[1]", [1]),
        ("[12]", [12]),
        ("[12_12]", [1212]),
        ("[0x12]", [18]),
        ("[0xab]", [171]),
        ("[0xAB]", [171]),
        ("[0X12]", [18]),
        ("[0Xab]", [171]),
        ("[0XAB]", [171]),
        ("[01234]", [668]),
        ("[0o1234]", [668]),
        ("[0O1234]", [668]),
        ("[0b1111]", [15]),
        ("[0B1111]", [15]),
        ("[-0]", [-0]),
        ("[-1]", [-1]),
        ("[-12]", [-12]),
        ("[-12_12]", [-1212]),
        ("[-0x12]", [-18]),
        ("[-0xab]", [-171]),
        ("[-0xAB]", [-171]),
        ("[-0X12]", [-18]),
        ("[-0Xab]", [-171]),
        ("[-0XAB]", [-171]),
        ("[-01234]", [-668]),
        ("[-0o1234]", [-668]),
        ("[-0O1234]", [-668]),
        ("[-0b1111]", [-15]),
        ("[-0B1111]", [-15]),
    ],
)
def test_integer_numeric_values(in_data, expected_data):
    result = parse_js_object(in_data)
    assert result == expected_data


@pytest.mark.parametrize(
    "in_data, expected_data",
    [
        ("[0.32]", [0.32]),
        ("[-0.32]", [-0.32]),
        ("[.32]", [0.32]),
        ("[-.32]", [-0.32]),
        ("[12.]", [12.0]),
        ("[-12.]", [-12.0]),
        ("[12.32]", [12.32]),
        ("[-12.12]", [-12.12]),
        ("[3.1415926]", [3.1415926]),
        ("[.123456789]", [0.123456789]),
        ("[.0123]", [0.0123]),
        ("[0.0123]", [0.0123]),
        ("[-.0123]", [-0.0123]),
        ("[-0.0123]", [-0.0123]),
        ("[3.1E+12]", [3.1e12]),
        ("[3.1e+12]", [3.1e12]),
        ("[.1E-23]", [0.1e-23]),
        ("[.1e-23]", [0.1e-23]),
    ],
)
def test_float_numeric_values(in_data, expected_data):
    result = parse_js_object(in_data)
    assert result == expected_data


@pytest.mark.parametrize(
    "in_data, expected_data",
    [
        (
            """
            var obj = {
                // Comment
                x: "X", // Comment
            };
        """,
            {"x": "X"},
        ),
        (
            """
            var /* Comment */ obj = /* Comment */ {
                /* Comment */
                x: /* Comment */ "X", /* Comment */
            };
        """,
            {"x": "X"},
        ),
        (
            """[/*...*/1,2,3,/*...*/4,5,6]""",
            [1, 2, 3, 4, 5, 6],
        ),
        (
            '// [ \n{"a": 1}',
            {"a": 1},
        ),
        (
            '/* [ */ {"a": 1}',
            {"a": 1},
        ),
        (
            """
        // ...
        // ...
        // ...
        // ...
        [1, 2, 3]
        """,
            [1, 2, 3],
        ),
        (
            """
        /* ... */
        /* ... */
        /* ... */
        /*
            ...
        */
        /*
            ...
        */
        [1, 2, 3]
        """,
            [1, 2, 3],
        ),
        (
            """
        /* <![CDATA[ */
        var foo = ["<", "_", ">"];
        /* ]]> */
        """,
            ["<", "_", ">"],
        ),
    ],
)
def test_comments(in_data, expected_data):
    result = parse_js_object(in_data)
    assert result == expected_data


@pytest.mark.parametrize(
    "in_data, expected_data",
    [
        ('["Test\\nDrive"]\n{"Test": "Drive"}', [["Test\nDrive"], {"Test": "Drive"}]),
    ],
)
def test_jsonlines(in_data, expected_data):
    result = list(parse_js_objects(in_data))
    assert result == expected_data


@pytest.mark.parametrize(
    "in_data, expected_exception",
    [
        ("}{", ValueError),
        ("", ValueError),
        (None, ValueError),
    ],
)
def test_exceptions(in_data, expected_exception):
    with pytest.raises(expected_exception):
        parse_js_object(in_data)


@pytest.mark.parametrize(
    "in_data, expected_exception",
    [
        ("{whose: 's's', category_name: '>'}", ValueError),
    ],
)
def test_malformed_input(in_data, expected_exception):
    with pytest.raises(expected_exception):
        parse_js_object(in_data)


@pytest.mark.parametrize(
    "in_data, expected_exception, expected_exception_text",
    [
        (
            '{"test": """}',
            ValueError,
            "Error parsing input near character 13",
        ),
    ],
)
def test_error_messages(in_data, expected_exception, expected_exception_text):
    with pytest.raises(expected_exception, match=expected_exception_text):
        parse_js_object(in_data)


@pytest.mark.parametrize(
    "in_data, expected_data",
    [
        ('{\\"a\\": 12}', {"a": 12}),
    ],
)
def test_unicode_escape(in_data, expected_data):
    result = parse_js_object(in_data, unicode_escape=True)
    assert result == expected_data


@pytest.mark.parametrize(
    "in_data, expected_data",
    [
        ('["\n"]', ["\n"]),
        (
            "{'a': '\"\"', 'b': '\\\\', 'c': '\t\n'}",
            {"a": '""', "b": "\\", "c": "\t\n"},
        ),
        (
            """var myObj = {
                myMethod: function(params) {
                    // ...
                },
                myValue: 100
            }""",
            {
                "myMethod": "function(params) {\n                    // ...\n                }",
                "myValue": 100,
            },
        ),
    ],
)
def test_json_non_strict(in_data, expected_data):
    result = parse_js_object(in_data, loader_kwargs={"strict": False})
    assert result == expected_data


@pytest.mark.parametrize(
    "in_data, expected_data",
    [
        ("[]", []),
        ("[1, 2, 3]", [1, 2, 3]),
        ("var x = [1, 2, 3, 4, 5,]", [1, 2, 3, 4, 5]),
        ("{}", {}),
        ("{'a': 12, 'b': 13, 'c': 14}", {"a": 12, "b": 13, "c": 14}),
        ("var x = {'a': 12, 'b': 13, 'c': 14}", {"a": 12, "b": 13, "c": 14}),
    ],
)
def test_loader(in_data, expected_data):
    import ast

    result = parse_js_object(in_data, loader=ast.literal_eval)
    assert result == expected_data


@pytest.mark.parametrize(
    "in_data, expected_data",
    [
        ("", []),
        ("aaaaaaaaaaaaaaaa", []),
        ("         ", []),
        ("      {'a': 12}", [{"a": 12}]),
        ("[1, 2, 3, 4]xxxxxxxxxxxxxxxxxxxxxxxx", [[1, 2, 3, 4]]),
        ("[12] [13] [14]", [[12], [13], [14]]),
        ("[10] {'a': [1, 1, 1,]}", [[10], {"a": [1, 1, 1]}]),
        ("[1][1][1]", [[1], [1], [1]]),
        ("[1] [2] {'a': ", [[1], [2]]),
        ("[]", [[]]),
        ("[][][][]", [[], [], [], []]),
        ("{}", [{}]),
        ("{}{}{}{}", [{}, {}, {}, {}]),
        ("{{}}{{}}", []),
        ("[[]][[]]", [[[]], [[]]]),
        ("{am: 'ab'}\n{'ab': 'xx'}", [{"am": "ab"}, {"ab": "xx"}]),
        (
            'function(a, b, c){ /* ... */ }({"a": 12}, Null, [1, 2, 3])',
            [{}, {"a": 12}, [1, 2, 3]],
        ),
        ('{"a": 12, broken}{"c": 100}', [{"c": 100}]),
        ("[12,,,,21][211,,,][12,12][12,,,21]", [[12, 12]]),
    ],
)
def test_parse_json_objects(in_data, expected_data):
    result = list(parse_js_objects(in_data))
    assert result == expected_data


@pytest.mark.parametrize(
    "in_data, expected_data",
    [
        ("[1][][2]", [[1], [2]]),
        ("{'a': 12}{}{'b': 13}", [{"a": 12}, {"b": 13}]),
        ("[][][][][][][][][]", []),
        ("{}{}{}{}{}{}{}{}{}", []),
    ],
)
def test_parse_json_objects_without_empty(in_data, expected_data):
    result = list(parse_js_objects(in_data, omitempty=True))
    assert result == expected_data
