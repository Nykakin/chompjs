#include <stdio.h>
#include <Python.h>
#include "parser.h"

static PyObject* parse_python_object(PyObject *self, PyObject *args) {
    const char* string;
    size_t initial_stack_size = 10;
    if (!PyArg_ParseTuple(args, "s|n", &string, &initial_stack_size)) {
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
        // initial lexer status
        CAN_ADVANCE,
        // initial stack index
        0,
        // initial stack size
        initial_stack_size,
        // initial stack
        malloc(initial_stack_size*sizeof(Type))
    };

    while(lexer.lexer_status == CAN_ADVANCE) {
        advance(&lexer);
    }

    PyObject* ret = Py_BuildValue("s", lexer.output);
    free((char*)lexer.output);
    free((Type*)lexer.stack);
    if(lexer.lexer_status == ERROR) {
        char error_message[50];
        strcpy(error_message, "Parser error: ...");
        strncpy(error_message+17, lexer.input+lexer.input_position, 30);

        PyErr_SetString(PyExc_ValueError, error_message);
        return NULL;
    }
    return ret;
}

static PyMethodDef parser_methods[] = { 
    {   
        "parse", parse_python_object, METH_VARARGS,
        "Parse JavaScript object string"
    },  
    {NULL, NULL, 0, NULL}
};


#if PY_MAJOR_VERSION >= 3

static struct PyModuleDef parser_definition = { 
    PyModuleDef_HEAD_INIT,
    "_chompjs",
    "C extension for fast JavaScript object parsing",
    -1, 
    parser_methods
};

PyMODINIT_FUNC PyInit__chompjs(void) {
    Py_Initialize();
    return PyModule_Create(&parser_definition);
}

#else

PyObject* init_chompjs(void) {
    return Py_InitModule("_chompjs", parser_methods);
}

#endif
