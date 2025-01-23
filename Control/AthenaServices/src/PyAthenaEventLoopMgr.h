/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENASERVICES_PYATHENAEVENTLOOPMGR_H
#define ATHENASERVICES_PYATHENAEVENTLOOPMGR_H
/** @file PyAthenaEventLoopMgr.h
    @brief Implementation of the AthenaEventLoopMgr that allows selective and
    specific overrides in python. The idea is to maximize code-sharing
    with AthenaEventLoopMgr to reduce maintainance.

   @author Atlas
*/

#include "CxxUtils/checker_macros.h"
#include "AthenaEventLoopMgr.h"

// Gaudi
class ISvcLocator;

// Python
struct _object;
typedef _object PyObject;

// Standard
#include <string>


/** @class PyAthenaEventLoopMgr
    @brief Implementation of the AthenaEventLoopMgr that allows selective and
    specific overrides in python. The idea is to maximize code-sharing
    with AthenaEventLoopMgr to reduce maintainance.
*/
class ATLAS_NOT_THREAD_SAFE PyAthenaEventLoopMgr : public AthenaEventLoopMgr   {
public:
  /// Standard Constructor
  PyAthenaEventLoopMgr( const std::string& name, ISvcLocator* svcLoc );
  ~PyAthenaEventLoopMgr() {}

  /// outside access
  static PyAthenaEventLoopMgr* pointer();

  /// actual manager object
  PyObject* setManager( PyObject* );
  PyObject* getManager();

   virtual StatusCode initialize();
   virtual StatusCode finalize();

protected:
   /// Unimplemented features to keep Reflex happy
   PyAthenaEventLoopMgr(); // Not implemented
   PyAthenaEventLoopMgr( const PyAthenaEventLoopMgr& ); // Not implemented
   PyAthenaEventLoopMgr& operator = ( const PyAthenaEventLoopMgr& ); // Not implemented
	
  /// Run the algorithms for the current event
  virtual StatusCode executeAlgorithms(const EventContext&);

private:
   PyObject* m_manager;
};

#endif // !ATHENASERVICES_PYATHENAEVENTLOOPMGR_H
