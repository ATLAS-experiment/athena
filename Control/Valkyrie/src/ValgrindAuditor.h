/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef VALKYRIE_VALGRINDAUDITOR_H
#define VALKYRIE_VALGRINDAUDITOR_H

// STL/Boost includes
#include <string>
#include <vector>
#include <utility>
#include <boost/regex.hpp>

// FrameWork includes
#include "Gaudi/Auditor.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GAUDI_VERSION.h"

// Forward declaration
class INamedInterface;
class IValgrindSvc;

//////////////////////////////////////////////////////////////////////
/// Valgrind auditor.
///
/// Gaudi auditor to programmatically control valgrind.
/// Currently only callgrind controls are implemented.
/// Turns callgrind instrumentation on/off before/afterExecute.
///
/// @author Frank Winklmeier
//////////////////////////////////////////////////////////////////////

class ValgrindAuditor : public Gaudi::Auditor,
                        virtual public IIncidentListener
{
public:  
  ValgrindAuditor(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~ValgrindAuditor();
     
  virtual StatusCode initialize() override;

  /// Incident handler
  virtual void handle( const Incident& incident );

  /// \name Auditor hooks
  //@{
  virtual void before(const std::string& event, const std::string& name,
                      const EventContext& ctx) override;

  virtual void after(const std::string& event, const std::string& name,
                     const EventContext& ctx, const StatusCode& sc) override;
  //@}


  /// Start callgrind instrumentation
  virtual void do_beforeExecute(const std::string& name);

  /// Stop callgrind instrumentation
  virtual void do_afterExecute(const std::string& name);

public:
  /// Typedef for algorithm/event pair, e.g. ("MyAlg","initialize")
  typedef std::pair<boost::regex,std::string> NameEvt;


private:
  /// Handle to ValgrindSvc
  ServiceHandle<IValgrindSvc> m_valSvc;

  /// List of algorithms to profile
  std::vector<std::string> m_algs;

  /// List of auditor intervals to profile
  std::vector<std::string> m_intervals;
  
  /// Don't profile on the first N events
  unsigned int m_ignoreFirstNEvents;

  /// Dump profile after each interval
  bool m_dumpAfterEachInterval;
  
  /// Internal event counter for BeginEvent incident
  unsigned int m_eventCounter;

  /// Regular expressions for algorithm name matching
  std::vector<boost::regex> m_algsRegEx;

  /// Internal storage of intervals
  std::vector< std::pair<NameEvt,NameEvt> > m_hooks;

  void do_before(const std::string& name, const std::string& hook);
  void do_after(const std::string& name, const std::string& hook);
  StatusCode decodeIntervals();
    
  bool algMatch(const std::string& name);
}; 

#endif 
