/*
 * Copyright 2020-2026 Mariusz Obajtek. All rights reserved.
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

    struct Parser parser;
    init_parser(&parser, string);
    Py_BEGIN_ALLOW_THREADS 
    while(parser.parser_status == CAN_ADVANCE) {
        advance(&parser);
    }
    Py_END_ALLOW_THREADS

    PyObject* ret = Py_BuildValue("s#", parser.output.data, parser.output.index-1);
    release_parser(&parser);
    if(parser.parser_status == ERROR) {
        const char* msg_sting = "Error parsing input near character %d";
        size_t error_buffer_size = snprintf(
            NULL,
            0,
            msg_sting,
            parser.input_position
        );       
        char* error_buffer = malloc(error_buffer_size + 1);
        sprintf(
            error_buffer,
            msg_sting,
            parser.input_position - 1
        );
        PyErr_SetString(PyExc_ValueError, error_buffer);
        free(error_buffer);
        return NULL;
    }
    return ret;
}

typedef struct {
    PyObject_HEAD
    struct Parser parser;
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
    init_parser(&json_iter_state->parser, string);

    return (PyObject* )json_iter_state;
}

static void json_iter_dealloc(JsonIterState* json_iter_state) {
    release_parser(&json_iter_state->parser);
    Py_TYPE(json_iter_state)->tp_free(json_iter_state);
}

static PyObject* json_iter_next(JsonIterState* json_iter_state) {
    Py_BEGIN_ALLOW_THREADS
    while(json_iter_state->parser.parser_status == CAN_ADVANCE) {
        advance(&json_iter_state->parser);
    }
    Py_END_ALLOW_THREADS

    if(json_iter_state->parser.output.index == 1) {
        return NULL;
    }
    PyObject* ret = Py_BuildValue(
        "s#",
        json_iter_state->parser.output.data,
        json_iter_state->parser.output.index-1
    );
    reset_parser_output(&json_iter_state->parser);
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
