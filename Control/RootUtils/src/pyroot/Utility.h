// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file RootUtils/src/pyroot/Utility.h
 * @author scott snyder, Wim Lavrijsen
 * @date Jul, 2015
 * @brief Utility code originally from pyroot.
 */


#ifndef ROOTUTILS_UTILITY_H
#define ROOTUTILS_UTILITY_H


#include "Python.h"
#include "TPython.h"
#include "TClass.h"


#if defined(ATLAS_HAVE_CPYCPPYY)
#include "CPyCppyy/PyException.h"
namespace RootUtils {
  using PyException = CPyCppyy::PyException;
}
#elif defined(ATLAS_HAVE_CPYRT)
#include "cpyrt/PyException.h"
namespace RootUtils {
using PyException = cppjit::cpyrt::PyException;
}
#else
#include <exception>
namespace RootUtils {
  struct PyException : public std::exception {
    const char* what() const noexcept override { return "Python exception"; }
  };
}
#endif  // ATLAS_HAVE_CPYCPPYY


namespace RootUtils {


int GetBuffer( PyObject* pyobject, char tc, int size, void*& buf, Bool_t check = kTRUE );
TClass* objectIsA (PyObject* obj);
bool setOwnership (PyObject* obj, bool flag);


}


#endif // not ROOTUTILS_UTILITY_H
