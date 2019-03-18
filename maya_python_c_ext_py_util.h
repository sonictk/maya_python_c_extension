#ifndef MAYA_PYTHON_C_EXT_PY_UTIL_H
#define MAYA_PYTHON_C_EXT_PY_UTIL_H

#include <Python.h>
#include <structmember.h>
#include <maya/MGlobal.h>


typedef struct
{
	PyObject_HEAD	// NOTE: (sonictk) This macro just brings in the refcount and a pointer to a type object.
	PyObject *first;
	PyObject *last;
	int number;
} TestObj;


static void TestObj_dealloc(TestObj *self)
{
	Py_XDECREF(self->first);
	Py_XDECREF(self->last);
	Py_TYPE(self)->tp_free((PyObject *)self);

	return;
}


static PyObject *TestObj_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	TestObj *self;

	self = (TestObj *)type->tp_alloc(type, 0);
	if (self != NULL) {
		self->first = PyString_FromString("");
		if (self->first == NULL) {
			Py_DECREF(self);

			return NULL;
		}

		self->last = PyString_FromString("");
		if (self->last == NULL) {
			Py_DECREF(self);

			return NULL;
		}

		self->number = 0;
	}

	return (PyObject *)self;
}


static int TestObj_init(TestObj *self, PyObject *args, PyObject *kwds)
{
	PyObject *first = NULL;
	PyObject *last = NULL;
	PyObject *tmp;

	static char *kwlist[] = {"first", "last", "number", NULL};

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "|OOi", kwlist, &first, &last, &self->number)) {
		return -1;
	}

	if (first) {
		tmp = self->first;
		Py_INCREF(first);
		self->first = first;
		Py_XDECREF(tmp);
	}

	if (last) {
		tmp = self->last;
		Py_INCREF(last);
		self->last = last;
		Py_XDECREF(tmp);
	}

	return 0;
}


static PyMemberDef TestObj_members[] = {
	{"first", T_OBJECT_EX, offsetof(TestObj, first), 0, "first name"},
	{"last", T_OBJECT_EX, offsetof(TestObj, last), 0, "last name"},
	{"number", T_INT, offsetof(TestObj, number), 0, "test number"},
	{NULL} // NOTE: (sonictk) Sentinel value
};


static PyObject *TestObj_name(TestObj *self)
{
	static PyObject *format = NULL;
	PyObject *args;
	PyObject *result;

	if (format == NULL) {
		format = PyString_FromString("%s %s");
		if (format == NULL) {
			return NULL;
		}
	}

	if (self->first == NULL) {
		PyErr_SetString(PyExc_AttributeError, "first");
		return NULL;
	}

	if (self->last == NULL) {
		PyErr_SetString(PyExc_AttributeError, "last");
		return NULL;
	}

	args = Py_BuildValue("OO", self->first, self->last);
	if (args == NULL) {
		return NULL;
	}

	result = PyString_Format(format, args);
	Py_DECREF(args);

	return result;
}


static PyMethodDef TestObj_methods[] = {
	{"name", (PyCFunction)TestObj_name, METH_NOARGS, "Return the name, combining the first and last name"},
	{NULL} // NOTE: (sonictk) Sentinel value
};


static PyTypeObject TestObjType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"maya_python_c_ext.TestObj",	   /* tp_name */ // NOTE: (sonictk) Use dotted name here, otherwise pydoc will not list the new type in the module documentation.
	sizeof(TestObj),			 /* tp_basicsize */ // NOTE: (sonictk) So that Python knows how much memory to allocate when calling PyObject_New().
	0,						   /* tp_itemsize */
	0,						   /* tp_dealloc */
	0,						   /* tp_print */
	0,						   /* tp_getattr */
	0,						   /* tp_setattr */
	0,						   /* tp_compare */
	0,						   /* tp_repr */
	0,						   /* tp_as_number */
	0,						   /* tp_as_sequence */
	0,						   /* tp_as_mapping */
	0,						   /* tp_hash */
	0,						   /* tp_call */
	0,						   /* tp_str */
	0,						   /* tp_getattro */
	0,						   /* tp_setattro */
	0,						   /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,	   /* tp_flags */
	"Test object",		       /* tp_doc */
	0,						   /* tp_traverse */
	0,						   /* tp_clear */
	0,						   /* tp_richcompare */
	0,						   /* tp_weaklistoffset */
	0,						   /* tp_iter */
	0,						   /* tp_iternext */
	TestObj_methods,		   /* tp_methods */
	TestObj_members,		   /* tp_members */
	0,						   /* tp_getset */
	0,						   /* tp_base */
	0,						   /* tp_dict */
	0,						   /* tp_descr_get */
	0,						   /* tp_descr_set */
	0,						   /* tp_dictoffset */
	(initproc)TestObj_init,    /* tp_init */
	0,						   /* tp_alloc */
	TestObj_new,			   /* tp_new */
};


static const char ADD_TO_ACTIVE_SELECTION_LIST_DOCSTRING[] = "Adds the specified object with the given name to the active selection list.";


static PyObject *pyAddToActiveSelectionList(PyObject *self, PyObject *args);


#endif /* MAYA_PYTHON_C_EXT_PY_UTIL_H */
