///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENAKERNEL_IVALGRINDSVC_H 
#define ATHENAKERNEL_IVALGRINDSVC_H 

// STL includes
#include <iosfwd>

// FrameWork includes
#include "GaudiKernel/IService.h"


/**
 * @class IValgrindSvc
 * @brief Abstract interface for ValgrindSvc.
 * @author Sebastien Binet
 */
class IValgrindSvc : virtual public IService
{ 
 
 public:
  /// Declare interface ID
  DeclareInterfaceID(IValgrindSvc, 2, 0);

  /// Destructor
  virtual ~IValgrindSvc();

  /// Start callgrind instrumentation
  virtual void callgrindStartInstrumentation() = 0;

  /// Stop callgrind instrumentation
  virtual void callgrindStopInstrumentation() = 0;

  /// Dump callgrind profiling stats
  virtual void callgrindDumpStats( std::ostream& out ) = 0;

  /// Toggle callgrind event collection
  virtual void callgrindToggleCollect() = 0;

  /// Do a leak check now
  virtual void valgrindDoLeakCheck() = 0;

  /// Number of created callgrind profiles
  virtual unsigned int profileCount() = 0;
  
};

#endif //> ATHENAKERNEL_IVALGRINDSVC_H
