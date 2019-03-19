#ifndef MAYA_PYTHON_C_EXT_PY_UTIL_H
#define MAYA_PYTHON_C_EXT_PY_UTIL_H

#include <Python.h>
#include <structmember.h> // NOTE: (sonictk) Needed for T_OBJECT_EX.
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
	// NOTE: (sonictk) XDECREF will take NULL into account while DECREF will not.
	Py_XDECREF(self->first);
	Py_XDECREF(self->last);

	// NOTE: (sonictk) Call the tp_free method to free the object's memory (in case
	// we're not freeing this type itself, but a instance of a subclass.)
	Py_TYPE(self)->tp_free((PyObject *)self);

	return;
}


/// Ensures that when a new object is created, that the first/last names are
/// initialized as empty strings. This is exposed in Python as the __new__() method.
/// Can be used to guarantee the initial values of instance variables. If this is
/// not required, can use PyType_GenericNew() instead as the __new__() method.
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


/// This is exposed as the __init__() method. Used to initialize an object after
/// it is created. Cannot guarantee that this is called, unlike the initializer.
/// __init__() can be called multiple times.
static int TestObj_init(TestObj *self, PyObject *args, PyObject *kwds)
{
	PyObject *first = NULL;
	PyObject *last = NULL;
	PyObject *tmp;

	static char *kwlist[] = {"first", "last", "number", NULL};

	// NOTE: (sonictk) Swap these two lines for generic object VS string only parsing
	// if (!PyArg_ParseTupleAndKeywords(args, kwds, "|OOi", kwlist, &first, &last, &self->number)) {
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "|SSi", kwlist, &first, &last, &self->number)) {
		return -1;
	}

	// NOTE: (sonictk) The type doesn't restrict the type of `first`, so it could
	// be any kind of object. It could have a destructor that causes code to be
	// executed that tries to access the `first` member. To guard against this,
	// usually re-assign members before decref-ing them.
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


/// Exposes the instance variables as attributes.
/// Does not provide a way to restrict the types of objects that can be assigned
/// to the Python attributes. Also doesn't stop the attributes from being deleted.
static PyMemberDef TestObj_members[] = {
	// {"first", T_OBJECT_EX, offsetof(TestObj, first), 0, "first name"},
	// {"last", T_OBJECT_EX, offsetof(TestObj, last), 0, "last name"},
	{"number", T_INT, offsetof(TestObj, number), 0, "test number"},
	{NULL} // NOTE: (sonictk) Sentinel value
};


static PyObject *TestObj_getfirst(TestObj *self, void *closure)
{
	Py_INCREF(self->first);

	return self->first;
}


static int TestObj_setfirst(TestObj *self, PyObject *value, void *closure)
{
	if (value == NULL) {
		PyErr_SetString(PyExc_AttributeError, "Cannot delete the attribute!");

		return -1;
	}

	if (!PyString_Check(value)) {
		PyErr_SetString(PyExc_TypeError, "The value must be a string!");

		return -1;
	}

	Py_DECREF(self->first);
	Py_INCREF(value);
	self->first = value;

	return 0;
}


// NOTE: (sonictk) Closure is unused here; it's meant to support cases where definition
// data is passed to the getter/setter i.e. can be used to allow a single set of
// setter/getter functions that decide the attribute to get/set based on data in
// the closure.
static PyObject *TestObj_getlast(TestObj *self, void *closure)
{
	Py_INCREF(self->last);

	return self->last;
}


static int TestObj_setlast(TestObj *self, PyObject *value, void *closure)
{
	if (value == NULL) {
		PyErr_SetString(PyExc_TypeError, "Cannot delete the attribute!");

		return -1;
	}

	if (!PyString_Check(value)) {
		PyErr_SetString(PyExc_TypeError, "The value must be a string!");

		return -1;
	}

	Py_DECREF(self->last);
	Py_INCREF(value);

	self->last = value;

	return 0;
}


static PyGetSetDef TestObj_getseters[] = {
	{"first", (getter)TestObj_getfirst, (setter)TestObj_setfirst, "first name", NULL},
	{"last", (getter)TestObj_getlast, (setter)TestObj_setlast, "last name", NULL},
	{NULL} // NOTE: (sonictk) Sentinel
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

	// if (self->first == NULL) {
	// 	PyErr_SetString(PyExc_AttributeError, "first");
	// 	return NULL;
	// }

	// if (self->last == NULL) {
	// 	PyErr_SetString(PyExc_AttributeError, "last");
	// 	return NULL;
	// }

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
	sizeof(TestObj),			       /* tp_basicsize */ // NOTE: (sonictk) So that Python knows how much memory to allocate when calling PyObject_New().
	0,						           /* tp_itemsize */
	(destructor)TestObj_dealloc,       /* tp_dealloc */
	0,						           /* tp_print */
	0,						           /* tp_getattr */
	0,						           /* tp_setattr */
	0,						           /* tp_compare */
	0,						           /* tp_repr */
	0,						           /* tp_as_number */
	0,						           /* tp_as_sequence */
	0,						           /* tp_as_mapping */
	0,						           /* tp_hash */
	0,						           /* tp_call */
	0,						           /* tp_str */
	0,						           /* tp_getattro */
	0,						           /* tp_setattro */
	0,						           /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE,	   /* tp_flags */
	"Test object",		               /* tp_doc */
	0,						           /* tp_traverse */
	0,						           /* tp_clear */
	0,						           /* tp_richcompare */
	0,						           /* tp_weaklistoffset */
	0,						           /* tp_iter */
	0,						           /* tp_iternext */
	TestObj_methods,		           /* tp_methods */
	TestObj_members,		           /* tp_members */
	TestObj_getseters,		           /* tp_getset */
	0,						           /* tp_base */
	0,						           /* tp_dict */
	0,						           /* tp_descr_get */
	0,						           /* tp_descr_set */
	0,						           /* tp_dictoffset */
	(initproc)TestObj_init,            /* tp_init */
	0,						           /* tp_alloc */
	TestObj_new,			           /* tp_new */
};


static const char ADD_TO_ACTIVE_SELECTION_LIST_DOCSTRING[] = "Adds the specified object with the given name to the active selection list.";


static PyObject *pyAddToActiveSelectionList(PyObject *self, PyObject *args);


#endif /* MAYA_PYTHON_C_EXT_PY_UTIL_H */
