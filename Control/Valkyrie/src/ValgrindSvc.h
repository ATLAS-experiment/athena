///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// ValgrindSvc.h 
// Header file for class ValgrindSvc
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef ATHENASERVICES_VALGRINDSVC_H 
#define ATHENASERVICES_VALGRINDSVC_H 1

// STL includes
#include <string>
#include <iosfwd>

// FrameWork includes
#include "AthenaBaseComps/AthService.h"
#include "GaudiKernel/IIncidentListener.h"

// AthenaKernel includes
#include "AthenaKernel/IValgrindSvc.h"

// Forward declaration
class ISvcLocator;
class IAuditorSvc;

class ValgrindSvc : public extends<AthService,
                                   IValgrindSvc,
                                   IIncidentListener>
{ 
  ///////////////////////////////////////////////////////////////////
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
 public: 

  /// Constructor with parameters:
  ValgrindSvc( const std::string& name, ISvcLocator* pSvcLocator );

  /// Destructor: 
  virtual ~ValgrindSvc(); 

  /// Gaudi Service Implementation
  //@{
  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;
  //@}

  /// incident service handle for Begin/EndEvent
  virtual void handle( const Incident& incident ) override;

  /// Start callgrind instrumentation
  virtual void callgrindStartInstrumentation() override;

  /// Stop callgrind instrumentation
  virtual void callgrindStopInstrumentation() override;

  /// Dump callgrind profiling stats
  virtual void callgrindDumpStats( std::ostream& out ) override;

  /// Toggle callgrind event collection
  virtual void callgrindToggleCollect() override;
  
  /// Do a leak check now
  virtual void valgrindDoLeakCheck() override;
  
  /// Number of created callgrind profiles
  virtual unsigned int profileCount() override { return m_profileCounter; }


  /////////////////////////////////////////////////////////////////// 
  // Private data: 
  /////////////////////////////////////////////////////////////////// 
 private: 

  /// List of algorithms to profile
  /// If list is empty, profile between begin/end event
  std::vector<std::string> m_algs;

  /// List of auditor intervals to profile
  /// Syntax: "MessageSvc.initialize:MessageSvc.finalize"
  std::vector<std::string> m_intervals;
  
  /// Dump separate profile after each event
  bool m_dumpAfterEachEvent;

  /// Dump separate profile after each interval
  bool m_dumpAfterEachInterval;

  /// Don't profile on the first N events
  unsigned int m_ignoreFirstNEvents;

  /// List of incidents on which to create a profile dump
  std::vector<std::string> m_dumpAfterIncident;
  
  /// Internal event counter for BeginEvent incident
  unsigned int m_eventCounter;

  /// Counter of created profiles
  unsigned int m_profileCounter;
}; 

#endif //> ATHENASERVICES_VALGRINDSVC_H
