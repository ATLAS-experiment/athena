/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PERFMONCOMPS_PERFMONMTAUDITOR_H
#define PERFMONCOMPS_PERFMONMTAUDITOR_H

// STL includes
#include <string>

// Framework includes
#include "Gaudi/Auditor.h"
#include "GaudiKernel/ServiceHandle.h"

// Forward declaration
class IPerfMonMTSvc;

class PerfMonMTAuditor : public Gaudi::Auditor
{
  public:

    /// Constructor
    PerfMonMTAuditor(const std::string& name, ISvcLocator* pSvcLocator);

    /// Gaudi hooks
    virtual StatusCode initialize() override;

    /// Implement inherited methods from Auditor
    virtual void before(const std::string& event, const std::string& name,
                        const EventContext&) override;

    virtual void after(const std::string& event, const std::string& name,
                       const EventContext&, const StatusCode&) override;

  private:

    /// Handle to PerfMonMTSvc
    ServiceHandle< IPerfMonMTSvc > m_perfMonMTSvc;

}; // end PerfMonMTAuditor

#endif // PERFMONCOMPS_PERFMONMTAUDITOR_H
