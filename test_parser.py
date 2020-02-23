import unittest
import js_object_parser


class TestParser(unittest.TestCase):
    def test_one_field_dict(self):
        result = js_object_parser.parse_js_object("{'hello': 'world'}")
        self.assertEqual(result, {'hello': 'world'})

    def test_two_fields_dict(self):
        result = js_object_parser.parse_js_object("{'hello': 'world', 'my': 'master'}")
        self.assertEqual(result, {'hello': 'world', 'my': 'master'})

    def test_nested_dict(self):
        result = js_object_parser.parse_js_object(
            "{'hello': 'world', 'my': {'master': 'of Orion'}, 'test': 'xx'}"
        )
        self.assertEqual(result, {'hello': 'world', 'my': {'master': 'of Orion'}, 'test': 'xx'})

    def test_numbers(self):
        result = js_object_parser.parse_js_object("{'hello': 12, 'world': 10002.21}")
        self.assertEqual(result, {'hello': 12, 'world': 10002.21})

    def test_empty_dict(self):
        result = js_object_parser.parse_js_object("{}")
        self.assertEqual(result, {})

    def test_empty_list(self):
        result = js_object_parser.parse_js_object("[]")
        self.assertEqual(result, [])

    def test_nested_lists(self):
        result = js_object_parser.parse_js_object("[[[]]]")
        self.assertEqual(result, [[[]]])

    def test_nested_lists_with_value(self):
        result = js_object_parser.parse_js_object("[[[1]]]")
        self.assertEqual(result, [[[1]]])

    def test_single_list_item(self):
        result = js_object_parser.parse_js_object("[1]")
        self.assertEqual(result, [1])

    def test_multiple_list_items(self):
        result = js_object_parser.parse_js_object("[1, 2, 3, 4]")
        self.assertEqual(result, [1, 2, 3, 4])

    def test_multiple_list_items_string(self):
        result = js_object_parser.parse_js_object("['h', 'e', 'l', 'l', 'o']")
        self.assertEqual(result, ['h', 'e', 'l', 'l', 'o'])

    def test_dict_with_lists(self):
        result = js_object_parser.parse_js_object("{'hello': [], 'world': [0]}")
        self.assertEqual(result, {'hello': [], 'world': [0]})

    def test_dict_with_multiple_element_list(self):
        result = js_object_parser.parse_js_object("{'hello': [1, 2, 3, 4]}")
        self.assertEqual(result, {'hello': [1, 2, 3, 4]})

    def test_list_of_dicts(self):
        result = js_object_parser.parse_js_object("[{'a':12}, {'b':33}]")
        self.assertEqual(result, [{'a': 12}, {'b': 33}])

    def test_non_quoted_identifier(self):
        result = js_object_parser.parse_js_object("{abcdefghijklmnopqrstuvwxyz: 12}")
        self.assertEqual(result, {"abcdefghijklmnopqrstuvwxyz": 12})

    def test_special_fields(self):
        result = js_object_parser.parse_js_object("{'a': true, 'b': false, 'c': null}")
        self.assertEqual(result, {'a': True, 'b': False, 'c': None})

    def test_quoted_strings(self):
        result = js_object_parser.parse_js_object("{'a': '123\\'456'}")
        self.assertEqual(result, {'a': "123'456"})

    def test_multiple_identifiers(self):
        result = js_object_parser.parse_js_object("{a:1,b:1,c:1,d:1,e:1,f:1,g:1,h:1,i:1,j:1}")
        self.assertEqual(result, {k: 1 for k in 'abcdefghij'})

    def test_depth(self):
        result = js_object_parser.parse_js_object("[[[[[[[[[[[[[[[1]]]]]]]]]]]]]]]")
        self.assertEqual(result, [[[[[[[[[[[[[[[1]]]]]]]]]]]]]]])

if __name__ == '__main__':
    unittest.main()
