/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef VTUNEAUDITOR_H
#define VTUNEAUDITOR_H

// STL include(s)
#include <vector>
#include <string>

// Framework include(s)
#include "Gaudi/Auditor.h"
#include "GaudiKernel/ServiceHandle.h"
#include "PerfMonVTune/IVTuneProfilerSvc.h"

class VTuneAuditor : public Gaudi::Auditor
{

  public:

    /// Constructor
    VTuneAuditor(const std::string& name, ISvcLocator* pSvcLocator);

    /// Gaudi hooks
    virtual StatusCode initialize() override;

    /// Implement inherited methods from Auditor
    virtual void before(const std::string& event, const std::string& name,
                        const EventContext&) override;

    virtual void after(const std::string& event, const std::string& name,
                       const EventContext&, const StatusCode&) override;

  private:

    /// Handle to VTuneProfilerSvc
    ServiceHandle< IVTuneProfilerSvc > m_vtuneProfilerSvc;

    /// Property: List of algorithms to profile
    std::vector<std::string> m_algs;

}; // end VTuneAuditor


#endif // VTUNEAUDITOR_H
