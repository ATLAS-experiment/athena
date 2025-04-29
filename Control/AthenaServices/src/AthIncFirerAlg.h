/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHENASERVICES_ATH_INCIDENT_FIRER_ALG_H
#define ATHENASERVICES_ATH_INCIDENT_FIRER_ALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "Gaudi/Property.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/ServiceHandle.h"

#include <vector>
#include <string>


/**
 * Algorithm to fire given list of incidents on execute.
 *
 * By default asynchronous incidents are fired, except if "FireSerial" is set to True.
 */
class AthIncFirerAlg : public AthReentrantAlgorithm {
public:
  AthIncFirerAlg(const std::string& name, ISvcLocator* pSvcLocator);
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

private:
  Gaudi::Property<std::vector<std::string>> m_incidents{this, "Incidents", {}, "List of incidents to fire"};
  Gaudi::Property<bool> m_serial{this, "FireSerial", false, "Whether to fire serial incidents"};

  ServiceHandle<IIncidentSvc> m_incSvc;
};

#endif
