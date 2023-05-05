//! \file
/*
**  Copyright (C) - Triton
**
**  This program is under the terms of the Apache License 2.0.
*/

#include <triton/pythonBindings.hpp>
#include <triton/pythonUtils.hpp>
#include <triton/pythonXFunctions.hpp>
#include <triton/cpuSize.hpp>



/*! \page py_CPUSIZE_page CPUSIZE
    \brief [**python api**] All information about the CPUSIZE Python namespace.

\tableofcontents

\section CPUSIZE_py_description Description
<hr>

According to the CPU architecture, the CPUSIZE namespace contains all kinds of sizes.

\section CPUSIZE_py_api Python API - Items of the CPUSIZE namespace
<hr>

- **CPUSIZE.BYTE**<br>
Returns `1`

- **CPUSIZE.BYTE_BIT**<br>
Returns `8`

- **CPUSIZE.WORD**<br>
Returns `2`

- **CPUSIZE.WORD_BIT**<br>
Returns `16`

- **CPUSIZE.DWORD**<br>
Returns `4`

- **CPUSIZE.DWORD_BIT**<br>
Returns `32`

- **CPUSIZE.QWORD**<br>
Returns `8`

- **CPUSIZE.QWORD_BIT**<br>
Returns `64`

- **CPUSIZE.DQWORD**<br>
Returns `16`

- **CPUSIZE.DQWORD_BIT**<br>
Returns `128`

- **CPUSIZE.QQWORD**<br>
Returns `32`

- **CPUSIZE.QQWORD_BIT**<br>
Returns `256`

- **CPUSIZE.DQQWORD**<br>
Returns `64`

- **CPUSIZE.DQQWORD_BIT**<br>
Returns `512`

*/



namespace triton {
  namespace bindings {
    namespace python {

      void initCpuSizeNamespace(PyObject* cpuSizeDict) {
        PyDict_Clear(cpuSizeDict);

        xPyDict_SetItemString(cpuSizeDict, "BYTE",        PyLong_FromUint32(triton::size::byte));
        xPyDict_SetItemString(cpuSizeDict, "WORD",        PyLong_FromUint32(triton::size::word));
        xPyDict_SetItemString(cpuSizeDict, "DWORD",       PyLong_FromUint32(triton::size::dword));
        xPyDict_SetItemString(cpuSizeDict, "QWORD",       PyLong_FromUint32(triton::size::qword));
        xPyDict_SetItemString(cpuSizeDict, "DQWORD",      PyLong_FromUint32(triton::size::dqword));
        xPyDict_SetItemString(cpuSizeDict, "QQWORD",      PyLong_FromUint32(triton::size::qqword));
        xPyDict_SetItemString(cpuSizeDict, "DQQWORD",     PyLong_FromUint32(triton::size::dqqword));
        xPyDict_SetItemString(cpuSizeDict, "BYTE_BIT",    PyLong_FromUint32(triton::bitsize::byte));
        xPyDict_SetItemString(cpuSizeDict, "WORD_BIT",    PyLong_FromUint32(triton::bitsize::word));
        xPyDict_SetItemString(cpuSizeDict, "DWORD_BIT",   PyLong_FromUint32(triton::bitsize::dword));
        xPyDict_SetItemString(cpuSizeDict, "QWORD_BIT",   PyLong_FromUint32(triton::bitsize::qword));
        xPyDict_SetItemString(cpuSizeDict, "DQWORD_BIT",  PyLong_FromUint32(triton::bitsize::dqword));
        xPyDict_SetItemString(cpuSizeDict, "QQWORD_BIT",  PyLong_FromUint32(triton::bitsize::qqword));
        xPyDict_SetItemString(cpuSizeDict, "DQQWORD_BIT", PyLong_FromUint32(triton::bitsize::dqqword));
      }

    }; /* python namespace */
  }; /* bindings namespace */
}; /* triton namespace */
