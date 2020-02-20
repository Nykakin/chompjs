#include <stdio.h>
#include <Python.h>
#include "parser.h"

static PyObject* parse_python_object(PyObject *self, PyObject *args) {
    const char* string;
    if (!PyArg_ParseTuple(args, "s", &string)) {
        return NULL;
    }

    struct Lexer lexer = {
        string,
        malloc(strlen(string)),
        0,
        0,
        {begin},
        1,
        0,
    };

    while(lexer.can_advance) {
        advance(&lexer);
    }
    putchar('\n');
    fflush(stdout);

    PyObject* ret = Py_BuildValue("s", lexer.output);
    free((char*)lexer.output);
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
