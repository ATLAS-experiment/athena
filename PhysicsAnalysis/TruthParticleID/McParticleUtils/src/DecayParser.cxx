/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////// 
// DecayParser.cxx 
// Implementation file for class DecayParser
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 

// Python includes
#include "Python.h"
#include "RootUtils/PyGetString.h"

// STL includes
#include <iostream>
#include <list>
#include <stdexcept>
#include <sstream>

// McParticleUtils includes
#include "McParticleUtils/DecayParser.h"



namespace {
  PyObject *fetch_py_parse_fct();
  bool py_to_cpp (PyObject* candidates,
		  std::vector<McUtils::Strings>& parsed);
}

/////////////////////////////////////////////////////////////////// 
/// Public methods: 
/////////////////////////////////////////////////////////////////// 

/// Constructors
////////////////

DecayParser::DecayParser( const std::string& cmd ) :
  m_parseFct (0),
  m_parents  ( ),
  m_children ( )
{
  m_parseFct = ::fetch_py_parse_fct();
  parse(cmd);
}


/// Destructor
///////////////
DecayParser::~DecayParser() 
{
  Py_XDECREF (m_parseFct);
}

/////////////////////////////////////////////////////////////////// 
/// Const methods: 
///////////////////////////////////////////////////////////////////
void DecayParser::dump() const
{
  std::cout << "--- Parents ---" << std::endl;
  printMcUtilsStrings( m_parents );

  std::cout << "--- Children ---" << std::endl;
  printMcUtilsStrings( m_children );
}

int DecayParser::pdgId( const std::string& pdgIdString ) const
{
  int pdgID = 0;
  int iPDG = 0;
  std::stringstream( pdgIdString ) >> iPDG;
  pdgID = iPDG;

  return pdgID;
}
/////////////////////////////////////////////////////////////////// 
/// Non-const methods: 
/////////////////////////////////////////////////////////////////// 
void DecayParser::parse( const std::string& inputCmd ) 
{
  if ( inputCmd.empty() ) {
    return;
  }

  // Reset the parents and children lists
  m_parents.clear();
  m_children.clear();



  // real parsing takes place now.
  PyObject *res = PyObject_CallFunction (m_parseFct,
					 (char*)"s",
					 inputCmd.c_str());
  if (!res) {
    Py_XDECREF (res);
    std::string error = "problem while parsing command [" + inputCmd +"]";
    throw std::runtime_error (error);
  }
  
  if (!PyTuple_Check (res)) {
    Py_DECREF (res);
    std::string error = "expected a python tuple";
    throw std::runtime_error (error);
  }

  if (PyTuple_GET_SIZE (res) != 3) {
    Py_DECREF (res);
    std::string error = "expected a python tuple of size 3";
    throw std::runtime_error (error);
  }

  PyObject *sc = PyTuple_GET_ITEM (res, 0);
  Py_XINCREF (sc);
  if (!sc || !PyLong_Check (sc)) {
    Py_XDECREF (sc);
    Py_DECREF  (res);
    std::string error = "corrupted return code";
    throw std::runtime_error (error);
  }

  Py_ssize_t status = PyLong_AsSsize_t (sc);
  if (status != 0) {
    Py_DECREF (sc);
    Py_DECREF (res);
    std::string error = "failed to parse command ["+inputCmd+"]";
    throw std::runtime_error (error);
  }
  Py_DECREF (sc);

  PyObject *parents = PyTuple_GET_ITEM (res, 1);
  Py_XINCREF (parents);
  if (!parents) {
    Py_DECREF (res);
    std::string error = "corrupted parents' list";
    throw std::runtime_error (error);
  }

  PyObject *children= PyTuple_GET_ITEM (res, 2);
  Py_XINCREF (children);
  if (!children) {
    Py_DECREF (parents);
    Py_DECREF (res);
    std::string error = "corrupted children' list";
    throw std::runtime_error (error);
  }
  Py_DECREF (res);

  if (parents==Py_None && children==Py_None) {
    // special case of a single arrow without any parent nor child :
    // this decay pattern will select every single vertex
    Py_DECREF (parents);
    Py_DECREF (children);
    return;
  }
  
  if (!py_to_cpp (parents, m_parents)) {
    Py_DECREF (parents);
    Py_DECREF (children);
    std::string error = "could not translate parents' list";
    throw std::runtime_error (error);
  }

  if (!py_to_cpp (children, m_children)) {
    Py_DECREF (parents);
    Py_DECREF (children);
    std::string error = "could not translate children' list";
    throw std::runtime_error (error);
  }

  return;
}


/////////////////////////////////////////////////////////////////// 
/// Protected methods: 
/////////////////////////////////////////////////////////////////// 


void 
DecayParser::printMcUtilsStrings( const std::vector<McUtils::Strings>& list ) const
{
  unsigned int iSlot = 0;
  for( std::vector<McUtils::Strings>::const_iterator itr = list.begin();
       itr != list.end();
       ++itr,++iSlot ) {
    std::stringstream iSlotStr;
    iSlotStr << iSlot;
    const McUtils::Strings::const_iterator candEnd = itr->end();
    std::cout << "slot #" << iSlotStr.str() << ": candidates= [ ";
    for( McUtils::Strings::const_iterator candidate = itr->begin();
	 candidate != candEnd;
	 ++candidate ) {
      std::cout << *candidate;
      if ( candidate+1 != candEnd ) {
	std::cout << " | ";
      }
    }
    std::cout << " ]" << std::endl;
  }
  return;
}


/////////////////////////////////////////////////////////////////// 
// Operators: 
///////////////////////////////////////////////////////////////////
// cppcheck-suppress operatorEqVarError; m_parseFct deliberately not copied. 
DecayParser & DecayParser::operator=(const DecayParser& rhs )
{
  if ( this != &rhs ) {
    m_parents      = rhs.m_parents;
    m_children     = rhs.m_children;
  }
  return *this;
}

namespace {

PyObject*
fetch_py_parse_fct()
{
  // need to ensure the python interpreter has been initialized...
  if (!Py_IsInitialized()) {
    Py_Initialize();
  }

  const std::string n = "McParticleUtils.DecayParser";
  PyObject *module = PyImport_ImportModule (const_cast<char*>(n.c_str()));
  if (!module || !PyModule_Check (module)) {
    Py_XDECREF (module);
    std::string error = "could not import module ["+n+"]";
    throw std::runtime_error (error);
  }

  const std::string fct_name = "py_parse";
  PyObject *fct = PyDict_GetItemString (PyModule_GetDict (module),
					const_cast<char*>(fct_name.c_str()));
  // borrowed ref.
  Py_XINCREF (fct);
  // don't need the module anymore
  Py_DECREF (module);

  if (!fct || !PyFunction_Check (fct)) {
    std::string error = "could not get '"+fct_name+"' from module ["+n+"] or not a function";
    throw std::runtime_error (error);
  }

  return fct;
}

bool 
py_to_cpp (PyObject* candidates,
	   std::vector<McUtils::Strings>& parsed)
{
  bool all_good = true;
  if (candidates==Py_None) {
    // nothing to do
    return true;
  }

  if (!PySequence_Check (candidates)) {
    return false;
  }
  Py_ssize_t isz = PySequence_Size (candidates);
  if (isz==-1) {
    return false;
  }
  parsed.resize (isz);

  for (Py_ssize_t i = 0; i!=isz; ++i) {
    PyObject *cand = PySequence_GetItem(candidates, i);
    if (!cand) {
      return false;
    }
    if (!PySequence_Check (cand)) {
      Py_DECREF (cand);
      return false;
    }
    Py_ssize_t jsz = PySequence_Size (cand);
    if (jsz==-1) {
      Py_DECREF (cand);
      return false;
    }

    parsed[i].resize(jsz);

    for (Py_ssize_t j = 0; j!=jsz; ++j) {
      PyObject *pdgid = PySequence_GetItem(cand, j);
      if (!pdgid) {
	Py_DECREF (cand);
	return false;
      }

      parsed[i][j] = RootUtils::PyGetString (pdgid).first;
    }

    Py_DECREF (cand);
  }
  return all_good;
}
} //> anon-namespace
