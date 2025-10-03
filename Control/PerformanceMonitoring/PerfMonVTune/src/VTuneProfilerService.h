/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef VTUNE_PROFILERSERVICE_H
#define VTUNE_PROFILERSERVICE_H

// STD include(s):
#include <atomic>
#include <memory>
#include <vector>
#include <string>

// Gaudi/Athena include(s):
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "AthenaBaseComps/AthService.h"

// Local include(s):
#include "VTuneProfileRunner.h"
#include "PerfMonVTune/IVTuneProfilerSvc.h"

// Fwd declerations
class IAuditorSvc;

class VTuneProfilerService : public extends<AthService,
                                            IVTuneProfilerSvc,
                                            IIncidentListener> {

  public:

      /// Standard Gaudi service constructor
      VTuneProfilerService( const std::string& name, ISvcLocator* svcloc );

      /// Standard Gaudi initialization function
      virtual StatusCode initialize() override;

      /// Resume profiling
      virtual StatusCode resumeProfiling() override;

      /// Pause profiling
      virtual StatusCode pauseProfiling() override;

      /// Is the profiling running at the moment?
      virtual bool isProfilingRunning() const override;

      /// Function handling incoming incidents
      virtual void handle( const Incident& inc ) override;

  private:

      /// Handle to the incident service
      ServiceHandle< IIncidentSvc > m_incidentSvc;

      /// Property: Event in which profiling should start
      int m_resumeEvent;

      /// Property: Event in which profiling should pause
      int m_pauseEvent;

      /// Property: List of algorithms to profile
      std::vector<std::string> m_algs;

      /// Unique ptr to the VTuneProfileRunner
      std::unique_ptr< VTuneProfileRunner > m_runner;

      /// Number of events processed so far
      std::atomic<int> m_processedEvents;

      std::mutex m_mutex;

}; // class IVTuneProfilerSvc

#endif // VTUNE_PROFILERSERVICE.H
