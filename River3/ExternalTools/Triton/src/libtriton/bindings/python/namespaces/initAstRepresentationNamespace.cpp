//! \file
/*
**  Copyright (C) - Triton
**
**  This program is under the terms of the Apache License 2.0.
*/

#include <triton/pythonBindings.hpp>
#include <triton/pythonUtils.hpp>
#include <triton/pythonXFunctions.hpp>
#include <triton/astRepresentation.hpp>



/*! \page py_AST_REPRESENTATION_page AST_REPRESENTATION
    \brief [**python api**] All information about the AST_REPRESENTATION Python namespace.

\tableofcontents

\section AST_REPRESENTATION_py_description Description
<hr>

The AST_REPRESENTATION namespace contains all kinds of AST representations.

\subsection AST_REPRESENTATION_py_example Example

~~~~~~~~~~~~~{.py}
>>> setAstRepresentationMode(AST_REPRESENTATION.PYTHON)
~~~~~~~~~~~~~

\section AST_REPRESENTATION_py_api Python API - Items of the AST_REPRESENTATION namespace
<hr>

- **AST_REPRESENTATION.SMT**<br>
Enabled, AST expressions will be represented in the SMT2-Lib syntax. This is the default mode.

- **AST_REPRESENTATION.PYTHON**<br>
Enabled, AST expressions will be represented in the Python syntax.


*/



namespace triton {
  namespace bindings {
    namespace python {

      void initAstRepresentationNamespace(PyObject* astRepresentationDict) {
        xPyDict_SetItemString(astRepresentationDict, "SMT",    PyLong_FromUint32(triton::ast::representations::SMT_REPRESENTATION));
        xPyDict_SetItemString(astRepresentationDict, "PYTHON", PyLong_FromUint32(triton::ast::representations::PYTHON_REPRESENTATION));
      }

    }; /* python namespace */
  }; /* bindings namespace */
}; /* triton namespace */
