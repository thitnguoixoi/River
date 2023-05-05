//! \file
/*
**  Copyright (C) - Triton
**
**  This program is under the terms of the Apache License 2.0.
*/

#include <triton/pythonObjects.hpp>
#include <triton/pythonUtils.hpp>
#include <triton/pythonXFunctions.hpp>
#include <triton/exceptions.hpp>
#include <triton/solverModel.hpp>



/*! \page py_SolverModel_page SolverModel
    \brief [**python api**] All information about the SolverModel Python object.

\tableofcontents

\section py_SolverModel_description Description
<hr>

This object is used to represent a model for an SMT solver.

~~~~~~~~~~~~~{.py}
>>> from __future__ import print_function
>>> from triton import TritonContext, ARCH, Instruction, REG

>>> ctxt = TritonContext()
>>> ctxt.setArchitecture(ARCH.X86_64)
>>> inst = Instruction()
>>> inst.setOpcode(b"\x48\x35\x44\x33\x22\x11") # xor rax, 0x11223344

>>> symvar = ctxt.symbolizeRegister(ctxt.registers.rax)
>>> print(symvar)
SymVar_0:64

>>> ctxt.processing(inst)
True
>>> print(inst)
0x0: xor rax, 0x11223344

>>> ast = ctxt.getAstContext()
>>> raxAst = ast.unroll(ctxt.getSymbolicRegister(ctxt.registers.rax).getAst())
>>> print(raxAst)
(bvxor SymVar_0 (_ bv287454020 64))

>>> astCtxt = ctxt.getAstContext()
>>> constraint = astCtxt.equal(raxAst, astCtxt.bv(0, raxAst.getBitvectorSize()))
>>> print(constraint)
(= (bvxor SymVar_0 (_ bv287454020 64)) (_ bv0 64))

>>> model = ctxt.getModel(constraint)
>>> print(model) #doctest: +ELLIPSIS
{0: SymVar_0:64 = 0x11223344}

>>> symvarModel =  model[symvar.getId()] # Model from the symvar's id
>>> print(symvarModel)
SymVar_0:64 = 0x11223344
>>> hex(symvarModel.getValue())
'0x11223344'

~~~~~~~~~~~~~

\section SolverModel_py_api Python API - Methods of the SolverModel class
<hr>

- <b>integer getId(void)</b><br>
Returns the id of the model. This id is the same as the variable id.

- <b>integer getValue(void)</b><br>
Returns the value of the model.

- <b>\ref py_SymbolicVariable_page getVariable(void)</b><br>
Returns the symbolic variable.

*/



namespace triton {
  namespace bindings {
    namespace python {

      //! SolverModel destructor.
      void SolverModel_dealloc(PyObject* self) {
        std::cout << std::flush;
        delete PySolverModel_AsSolverModel(self);
        Py_TYPE(self)->tp_free((PyObject*)self);
      }


      static PyObject* SolverModel_getId(PyObject* self, PyObject* noarg) {
        try {
          return PyLong_FromUsize(PySolverModel_AsSolverModel(self)->getId());
        }
        catch (const triton::exceptions::Exception& e) {
          return PyErr_Format(PyExc_TypeError, "%s", e.what());
        }
      }


      static PyObject* SolverModel_getValue(PyObject* self, PyObject* noarg) {
        try {
          return PyLong_FromUint512(PySolverModel_AsSolverModel(self)->getValue());
        }
        catch (const triton::exceptions::Exception& e) {
          return PyErr_Format(PyExc_TypeError, "%s", e.what());
        }
      }


      static PyObject* SolverModel_getVariable(PyObject* self, PyObject* noarg) {
        try {
          return PySymbolicVariable(PySolverModel_AsSolverModel(self)->getVariable());
        }
        catch (const triton::exceptions::Exception& e) {
          return PyErr_Format(PyExc_TypeError, "%s", e.what());
        }
      }


      #if !defined(IS_PY3_8) || !IS_PY3_8
      static int SolverModel_print(PyObject* self, void* io, int s) {
        std::cout << PySolverModel_AsSolverModel(self);
        return 0;
      }
      #endif


      static PyObject* SolverModel_str(PyObject* self) {
        try {
          std::stringstream str;
          str << PySolverModel_AsSolverModel(self);
          return PyStr_FromFormat("%s", str.str().c_str());
        }
        catch (const triton::exceptions::Exception& e) {
          return PyErr_Format(PyExc_TypeError, "%s", e.what());
        }
      }


      //! SolverModel methods.
      PyMethodDef SolverModel_callbacks[] = {
        {"getId",       SolverModel_getId,        METH_NOARGS,    ""},
        {"getValue",    SolverModel_getValue,     METH_NOARGS,    ""},
        {"getVariable", SolverModel_getVariable,  METH_NOARGS,    ""},
        {nullptr,       nullptr,                  0,              nullptr}
      };


      PyTypeObject SolverModel_Type = {
        PyVarObject_HEAD_INIT(&PyType_Type, 0)
        "SolverModel",                              /* tp_name */
        sizeof(SolverModel_Object),                 /* tp_basicsize */
        0,                                          /* tp_itemsize */
        (destructor)SolverModel_dealloc,            /* tp_dealloc */
        #if IS_PY3_8
        0,                                          /* tp_vectorcall_offset */
        #else
        (printfunc)SolverModel_print,               /* tp_print */
        #endif
        0,                                          /* tp_getattr */
        0,                                          /* tp_setattr */
        0,                                          /* tp_compare */
        (reprfunc)SolverModel_str,                  /* tp_repr */
        0,                                          /* tp_as_number */
        0,                                          /* tp_as_sequence */
        0,                                          /* tp_as_mapping */
        0,                                          /* tp_hash */
        0,                                          /* tp_call */
        (reprfunc)SolverModel_str,                  /* tp_str */
        0,                                          /* tp_getattro */
        0,                                          /* tp_setattro */
        0,                                          /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT,                         /* tp_flags */
        "SolverModel objects",                      /* tp_doc */
        0,                                          /* tp_traverse */
        0,                                          /* tp_clear */
        0,                                          /* tp_richcompare */
        0,                                          /* tp_weaklistoffset */
        0,                                          /* tp_iter */
        0,                                          /* tp_iternext */
        SolverModel_callbacks,                      /* tp_methods */
        0,                                          /* tp_members */
        0,                                          /* tp_getset */
        0,                                          /* tp_base */
        0,                                          /* tp_dict */
        0,                                          /* tp_descr_get */
        0,                                          /* tp_descr_set */
        0,                                          /* tp_dictoffset */
        0,                                          /* tp_init */
        0,                                          /* tp_alloc */
        0,                                          /* tp_new */
        0,                                          /* tp_free */
        0,                                          /* tp_is_gc */
        0,                                          /* tp_bases */
        0,                                          /* tp_mro */
        0,                                          /* tp_cache */
        0,                                          /* tp_subclasses */
        0,                                          /* tp_weaklist */
        0,                                          /* tp_del */
        #if IS_PY3
        0,                                          /* tp_version_tag */
        0,                                          /* tp_finalize */
        #if IS_PY3_8
        0,                                          /* tp_vectorcall */
        0,                                          /* bpo-37250: kept for backwards compatibility in CPython 3.8 only */
        #endif
        #else
        0                                           /* tp_version_tag */
        #endif
      };


      PyObject* PySolverModel(const triton::engines::solver::SolverModel& model) {
        SolverModel_Object* object;

        PyType_Ready(&SolverModel_Type);
        object = PyObject_NEW(SolverModel_Object, &SolverModel_Type);
        if (object != NULL)
          object->model = new triton::engines::solver::SolverModel(model);

        return (PyObject*)object;
      }

    }; /* python namespace */
  }; /* bindings namespace */
}; /* triton namespace */
