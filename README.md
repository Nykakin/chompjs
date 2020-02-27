```bash
$ python
Python 3.7.3 (default, Oct  7 2019, 12:56:13) 
[GCC 8.3.0] on linux
Type "help", "copyright", "credits" or "license" for more information.
>>> import chompjs
>>> import pprint
>>> json_data = chompjs.parse_js_object("""{
...     '152065' : {
...         canonicalURL: 'https://www.chewy.com/living-world-cuttlebone-bird-treat-2/dp/152065',
...         ajaxURL: `/living-world-cuttlebone-bird-treat-2/dp/152065?features`,
...         sku: 124945,
...         images: [
...             '//img.chewy.com/is/image/catalog/124945_MAIN._AC_SL400_V1495567031_.jpg',
...             '//img.chewy.com/is/image/catalog/124945_PT2._AC_SL320_V1497994333_.jpg',
...         ],
...         price: '$1.69'
...     },
...     '131457' : {
...         canonicalURL: 'https://www.chewy.com/living-world-cuttlebone-bird-treat/dp/131457',
...         ajaxURL: `/living-world-cuttlebone-bird-treat/dp/131457?features`,
...         sku: 103970,
...         images: [
...             '//img.chewy.com/is/catalog/103970._AC_SL400_V1469015482_.jpg',
...             '//img.chewy.com/is/image/catalog/103970_PT1._AC_SL320_V1518213672_.jpg',
...         ],
...         price: '$5.91'
...     }
... }""")

>>> pprint.pprint(json_data)
{'131457': {'ajaxURL': '/living-world-cuttlebone-bird-treat/dp/131457?features',
            'canonicalURL': 'https://www.chewy.com/living-world-cuttlebone-bird-treat/dp/131457',
            'images': ['//img.chewy.com/is/catalog/103970._AC_SL400_V1469015482_.jpg',
                       '//img.chewy.com/is/image/catalog/103970_PT1._AC_SL320_V1518213672_.jpg'],
            'price': '$5.91',
            'sku': 103970},
 '152065': {'ajaxURL': '/living-world-cuttlebone-bird-treat-2/dp/152065?features',
            'canonicalURL': 'https://www.chewy.com/living-world-cuttlebone-bird-treat-2/dp/152065',
            'images': ['//img.chewy.com/is/image/catalog/124945_MAIN._AC_SL400_V1495567031_.jpg',
                       '//img.chewy.com/is/image/catalog/124945_PT2._AC_SL320_V1497994333_.jpg'],
            'price': '$1.69',
            'sku': 124945}}

```

### Installation

```bash
$ python3 -m venv venv
$ . venv/bin/activate
# pip install chompjs
```

To run unittests
```
$ python -m unittest
```
