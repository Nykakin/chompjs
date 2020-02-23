#include <stdio.h>
#include <Python.h>
#include "parser.h"

static PyObject* parse_python_object(PyObject *self, PyObject *args) {
    const char* string;
    if (!PyArg_ParseTuple(args, "s", &string)) {
        return NULL;
    }

    struct Lexer lexer = {
        // input js object
        string,
        // output JSON
        //
        // alloc twice the size of input because characters are added when
        // identifiers are quoted, e.g. from '{a:1}' to  '{"a":1}'
        // so output might be larger than input, especially for malicious input
        // such as '{a:1,b:1,c:1,d:1,e:1,f:1,g:1,h:1,i:1,j:1}' is translated to
        // '{"a":1,"b":1,"c":1,"d":1,"e":1,"f":1,"g":1,"h":1,"i":1}'
        malloc(2*strlen(string)),
        // input string position
        0,
        // output string position
        0,
        // initial state
        {begin},
        // can_advance
        1,
        // initial stack index
        0,
        // initial stack size
        10,
        // initial stack
        malloc(10*sizeof(type))
    };

    while(lexer.can_advance) {
        advance(&lexer);
    }

    PyObject* ret = Py_BuildValue("s", lexer.output);
    free((char*)lexer.output);
    free((type*)lexer.stack);
    return ret;
}

static PyMethodDef parser_methods[] = { 
    {   
        "parse", parse_python_object, METH_VARARGS,
        "Parse JavaScript object string"
    },  
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef parser_definition = { 
    PyModuleDef_HEAD_INIT,
    "hello",
    "C extension for fast JavaScript object parsing",
    -1, 
    parser_methods
};

PyMODINIT_FUNC PyInit_hello(void) {
    Py_Initialize();
    return PyModule_Create(&parser_definition);
}
