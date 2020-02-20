```bash
$ python
Python 3.7.3 (default, Oct  7 2019, 12:56:13) 
[GCC 8.3.0] on linux
Type "help", "copyright", "credits" or "license" for more information.
>>> import js_object_parser
>>> js_object_parser.parse_js_object("""{
...         
...         '152065' : {
...             canonicalURL: 'https://www.chewy.com/living-world-cuttlebone-bird-treat-2/dp/152065',
...             ajaxURL: `/living-world-cuttlebone-bird-treat-2/dp/152065?features`,
...             sku: 124945,
...             images: [
...                 '//img.chewy.com/is/image/catalog/124945_MAIN._AC_SL400_V1495567031_.jpg',
...                 
...                         '//img.chewy.com/is/image/catalog/124945_PT2._AC_SL320_V1497994333_.jpg',
...                      ],
...             price: '$1.69'
...         },
...         '131457' : {
...             canonicalURL: 'https://www.chewy.com/living-world-cuttlebone-bird-treat/dp/131457',
...             ajaxURL: `/living-world-cuttlebone-bird-treat/dp/131457?features`,
...             sku: 103970,
...             images: [
...                 '//img.chewy.com/is/catalog/103970._AC_SL400_V1469015482_.jpg',
...                 
...                         '//img.chewy.com/is/image/catalog/103970_PT1._AC_SL320_V1518213672_.jpg',
...                      ],
...             price: '$5.91'
...         }
...     }""")

{'152065': {'canonicalURL': 'https://www.chewy.com/living-world-cuttlebone-bird-treat-2/dp/152065', 'ajaxURL': '/living-world-cuttlebone-bird-treat-2/dp/152065?features', 'sku': 124945, 'images': ['//img.chewy.com/is/image/catalog/124945_MAIN._AC_SL400_V1495567031_.jpg', '//img.chewy.com/is/image/catalog/124945_PT2._AC_SL320_V1497994333_.jpg'], 'price': '$1.69'}, '131457': {'canonicalURL': 'https://www.chewy.com/living-world-cuttlebone-bird-treat/dp/131457', 'ajaxURL': '/living-world-cuttlebone-bird-treat/dp/131457?features', 'sku': 103970, 'images': ['//img.chewy.com/is/catalog/103970._AC_SL400_V1469015482_.jpg', '//img.chewy.com/is/image/catalog/103970_PT1._AC_SL320_V1518213672_.jpg'], 'price': '$5.91'}}
```

### Installation

```bash
(venv) mariusz ~/Documents/Projekty/JSObjectParser $ python3 -m venv venv
(venv) mariusz ~/Documents/Projekty/JSObjectParser $ . venv/bin/activate
(venv) mariusz ~/Documents/Projekty/JSObjectParser $ python build
python: can't open file 'build': [Errno 2] No such file or directory
(venv) mariusz ~/Documents/Projekty/JSObjectParser $ python setup.py build
running build
running build_ext
building 'hello' extension
creating build
creating build/temp.linux-x86_64-3.7
x86_64-linux-gnu-gcc -pthread -Wno-unused-result -Wsign-compare -DNDEBUG -g -fwrapv -O2 -Wall -g -fstack-protector-strong -Wformat -Werror=format-security -g -flto -fuse-linker-plugin -ffat-lto-objects -g -fstack-protector-strong -Wformat -Werror=format-security -Wdate-time -D_FORTIFY_SOURCE=2 -fPIC -I/home/mariusz/Documents/Projekty/JSObjectParser/venv/include -I/usr/include/python3.7m -c module.c -o build/temp.linux-x86_64-3.7/module.o
x86_64-linux-gnu-gcc -pthread -Wno-unused-result -Wsign-compare -DNDEBUG -g -fwrapv -O2 -Wall -g -fstack-protector-strong -Wformat -Werror=format-security -g -flto -fuse-linker-plugin -ffat-lto-objects -g -fstack-protector-strong -Wformat -Werror=format-security -Wdate-time -D_FORTIFY_SOURCE=2 -fPIC -I/home/mariusz/Documents/Projekty/JSObjectParser/venv/include -I/usr/include/python3.7m -c parser.c -o build/temp.linux-x86_64-3.7/parser.o
creating build/lib.linux-x86_64-3.7
x86_64-linux-gnu-gcc -pthread -shared -Wl,-O1 -Wl,-Bsymbolic-functions -Wl,-Bsymbolic-functions -Wl,-z,relro -Wl,-Bsymbolic-functions -Wl,-z,relro -g -fstack-protector-strong -Wformat -Werror=format-security -Wdate-time -D_FORTIFY_SOURCE=2 build/temp.linux-x86_64-3.7/module.o build/temp.linux-x86_64-3.7/parser.o -o build/lib.linux-x86_64-3.7/hello.cpython-37m-x86_64-linux-gnu.so
(venv) mariusz ~/Documents/Projekty/JSObjectParser $ python setup.py install
running install
running build
running build_ext
running install_lib
copying build/lib.linux-x86_64-3.7/hello.cpython-37m-x86_64-linux-gnu.so -> /home/mariusz/Documents/Projekty/JSObjectParser/venv/lib/python3.7/site-packages
running install_egg_info
Writing /home/mariusz/Documents/Projekty/JSObjectParser/venv/lib/python3.7/site-packages/js_object_parser-0.1.0.egg-info
```

To run unittests
```
$ python -m unittest
```
