/*
 * Copyright 2020-2024 Mariusz Obajtek. All rights reserved.
 * License: https://github.com/Nykakin/chompjs/blob/master/LICENSE
 */

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <stdio.h>
#include "parser.h"

static PyObject* parse_python_object(PyObject *self, PyObject *args) {
    const char* string;
    if (!PyArg_ParseTuple(args, "s", &string)) {
        return NULL;
    }

    struct Lexer lexer;
    init_lexer(&lexer, string);
    while(lexer.lexer_status == CAN_ADVANCE) {
        advance(&lexer);
    }

    PyObject* ret = Py_BuildValue("s#", lexer.output.data, lexer.output.index-1);
    release_lexer(&lexer);
    if(lexer.lexer_status == ERROR) {
        const char* msg_sting = "Error parsing input near character %d";
        size_t error_buffer_size = snprintf(
            NULL,
            0,
            msg_sting,
            lexer.input_position
        );       
        char* error_buffer = malloc(error_buffer_size + 1);
        sprintf(
            error_buffer,
            msg_sting,
            lexer.input_position - 1
        );
        PyErr_SetString(PyExc_ValueError, error_buffer);
        free(error_buffer);
        return NULL;
    }
    return ret;
}

typedef struct {
    PyObject_HEAD
    struct Lexer lexer;
} JsonIterState;

static PyObject* json_iter_new(PyTypeObject *type, PyObject *args, PyObject *kwargs) {
    JsonIterState* json_iter_state = (JsonIterState *)type->tp_alloc(type, 0);
    if (!json_iter_state) {
        return NULL;
    }

    const char* string;
    if (!PyArg_ParseTuple(args, "s", &string)) {
        return NULL;
    }
    init_lexer(&json_iter_state->lexer, string);

    return (PyObject* )json_iter_state;
}

static void json_iter_dealloc(JsonIterState* json_iter_state) {
    release_lexer(&json_iter_state->lexer);
    Py_TYPE(json_iter_state)->tp_free(json_iter_state);
}

static PyObject* json_iter_next(JsonIterState* json_iter_state) {
    while(json_iter_state->lexer.lexer_status == CAN_ADVANCE) {
        advance(&json_iter_state->lexer);
    }
    if(json_iter_state->lexer.output.index == 1) {
        return NULL;
    }
    PyObject* ret = Py_BuildValue(
        "s#",
        json_iter_state->lexer.output.data,
        json_iter_state->lexer.output.index-1
    );
    reset_lexer_output(&json_iter_state->lexer);
    return ret;
}

PyTypeObject JSONIter_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "json_iter",                    /* tp_name */
    sizeof(JsonIterState),          /* tp_basicsize */
    0,                              /* tp_itemsize */
    (destructor)json_iter_dealloc,  /* tp_dealloc */
    0,                              /* tp_print */
    0,                              /* tp_getattr */
    0,                              /* tp_setattr */
    0,                              /* tp_reserved */
    0,                              /* tp_repr */
    0,                              /* tp_as_number */
    0,                              /* tp_as_sequence */
    0,                              /* tp_as_mapping */
    0,                              /* tp_hash */
    0,                              /* tp_call */
    0,                              /* tp_str */
    0,                              /* tp_getattro */
    0,                              /* tp_setattro */
    0,                              /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,             /* tp_flags */
    0,                              /* tp_doc */
    0,                              /* tp_traverse */
    0,                              /* tp_clear */
    0,                              /* tp_richcompare */
    0,                              /* tp_weaklistoffset */
    PyObject_SelfIter,              /* tp_iter */
    (iternextfunc)json_iter_next,   /* tp_iternext */
    0,                              /* tp_methods */
    0,                              /* tp_members */
    0,                              /* tp_getset */
    0,                              /* tp_base */
    0,                              /* tp_dict */
    0,                              /* tp_descr_get */
    0,                              /* tp_descr_set */
    0,                              /* tp_dictoffset */
    0,                              /* tp_init */
    PyType_GenericAlloc,            /* tp_alloc */
    json_iter_new,                  /* tp_new */
};

static PyObject* parse_python_objects(PyObject *self, PyObject *args) {
    PyObject *obj = PyObject_CallObject((PyObject *) &JSONIter_Type, args);
    return obj;
}

static PyMethodDef parser_methods[] = { 
    {   
        "parse", parse_python_object, METH_VARARGS,
        "Extract JSON object from the string"
    },  
    {   
        "parse_objects", parse_python_objects, METH_VARARGS,
        "Iterate over all JSON objects in the string"
    },  
    {NULL, NULL, 0, NULL}
};


static struct PyModuleDef parser_definition = { 
    PyModuleDef_HEAD_INIT,
    "_chompjs",
    "C extension for fast JavaScript object parsing",
    -1, 
    parser_methods
};

PyMODINIT_FUNC PyInit__chompjs(void) {
    Py_Initialize();
    PyObject* module = PyModule_Create(&parser_definition);
    if (!module) {
        return NULL;
    }
    if (PyType_Ready(&JSONIter_Type) < 0) {
        return NULL;
    }
    return module;
}
