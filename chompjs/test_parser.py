# -*- coding: utf-8 -*-
from __future__ import unicode_literals

import unittest
from chompjs import parse_js_object


class TestParser(unittest.TestCase):
    def test_one_field_dict(self):
        result = parse_js_object("{'hello': 'world'}")
        self.assertEqual(result, {'hello': 'world'})

    def test_two_fields_dict(self):
        result = parse_js_object("{'hello': 'world', 'my': 'master'}")
        self.assertEqual(result, {'hello': 'world', 'my': 'master'})

    def test_nested_dict(self):
        result = parse_js_object(
            "{'hello': 'world', 'my': {'master': 'of Orion'}, 'test': 'xx'}"
        )
        self.assertEqual(result, {'hello': 'world', 'my': {'master': 'of Orion'}, 'test': 'xx'})

    def test_numbers(self):
        result = parse_js_object("{'hello': 12, 'world': 10002.21}")
        self.assertEqual(result, {'hello': 12, 'world': 10002.21})

    def test_empty_dict(self):
        result = parse_js_object("{}")
        self.assertEqual(result, {})

    def test_empty_list(self):
        result = parse_js_object("[]")
        self.assertEqual(result, [])

    def test_nested_lists(self):
        result = parse_js_object("[[[]]]")
        self.assertEqual(result, [[[]]])

    def test_nested_lists_with_value(self):
        result = parse_js_object("[[[1]]]")
        self.assertEqual(result, [[[1]]])

    def test_single_list_item(self):
        result = parse_js_object("[1]")
        self.assertEqual(result, [1])

    def test_multiple_list_items(self):
        result = parse_js_object("[1, 2, 3, 4]")
        self.assertEqual(result, [1, 2, 3, 4])

    def test_multiple_list_items_string(self):
        result = parse_js_object("['h', 'e', 'l', 'l', 'o']")
        self.assertEqual(result, ['h', 'e', 'l', 'l', 'o'])

    def test_dict_with_lists(self):
        result = parse_js_object("{'hello': [], 'world': [0]}")
        self.assertEqual(result, {'hello': [], 'world': [0]})

    def test_dict_with_multiple_element_list(self):
        result = parse_js_object("{'hello': [1, 2, 3, 4]}")
        self.assertEqual(result, {'hello': [1, 2, 3, 4]})

    def test_list_of_dicts(self):
        result = parse_js_object("[{'a':12}, {'b':33}]")
        self.assertEqual(result, [{'a': 12}, {'b': 33}])

    def test_non_quoted_identifier(self):
        result = parse_js_object("{abcdefghijklmnopqrstuvwxyz: 12}")
        self.assertEqual(result, {"abcdefghijklmnopqrstuvwxyz": 12})

    def test_special_fields(self):
        result = parse_js_object("{'a': true, 'b': false, 'c': null}")
        self.assertEqual(result, {'a': True, 'b': False, 'c': None})

    def test_quoted_strings(self):
        result = parse_js_object("{'a': '123\\'456'}")
        self.assertEqual(result, {'a': "123'456"})

    def test_multiple_identifiers(self):
        result = parse_js_object("{a:1,b:1,c:1,d:1,e:1,f:1,g:1,h:1,i:1,j:1}")
        self.assertEqual(result, {k: 1 for k in 'abcdefghij'})

    def test_depth(self):
        result = parse_js_object("[[[[[[[[[[[[[[[1]]]]]]]]]]]]]]]")
        self.assertEqual(result, [[[[[[[[[[[[[[[1]]]]]]]]]]]]]]])

    def test_unicode(self):
        result = parse_js_object("['\u00E9']")
        self.assertEqual(result, ['é'])

    def test_stack(self):
        result = parse_js_object(
            "{'a':[{'b':1},{'c':[{'d':{'f':{'g':[1,2]}}},{'e':1}]}]}",
            initial_stack_size=1,
        )
        self.assertEqual(result, {'a': [{'b': 1}, {'c': [{'d': {'f': {'g': [1, 2]}}}, {'e': 1}]}]})

    def test_negative_number_literals(self):
        result = parse_js_object('{"a": -12, "b": - 5}')
        self.assertEqual(result, {'a': -12, 'b': -5})

    def test_number_literals_with_separators(self):
        result = parse_js_object('{"a": 12_12}')
        self.assertEqual(result, {'a': 1212})

    def test_scientific_notation_number_literal(self):
        result = parse_js_object('{"a": 3.125e7}')
        self.assertEqual(result, {'a': 3.125e7})

    def test_after_text(self):
        result = parse_js_object('{"a": {"b": [12, 13, 14]}}text text')
        self.assertEqual(result, {"a": {"b": [12, 13, 14]}})

    def test_before_text(self):
        result = parse_js_object('var test = {"a": {"b": [12, 13, 14]}}')
        self.assertEqual(result, {"a": {"b": [12, 13, 14]}})


class TestParserExceptions(unittest.TestCase):
    def test_invalid_input(self):
        with self.assertRaises(ValueError):
            parse_js_object('}{')

    def test_empty_input(self):
        with self.assertRaises(ValueError):
            parse_js_object('')

    def test_none_input(self):
        with self.assertRaises(ValueError):
            parse_js_object(None)


class TestUnicodeEscape(unittest.TestCase):
    def test_unicode_escape(self):
        result = parse_js_object('{\\\"a\\\": 12}', unicode_escape=True)
        self.assertEqual(result, {'a': 12})


if __name__ == '__main__':
    unittest.main()
