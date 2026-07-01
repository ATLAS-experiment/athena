// emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JETREC_GLOBALEVENTSETUP_H
#define JETREC_GLOBALEVENTSETUP_H


#include "GaudiKernel/IIncidentListener.h"
#include "AthenaBaseComps/AthAlgorithm.h"
#include "CxxUtils/checker_macros.h"

#include <string>


/**@class JetGlobalEventSetup
 * @brief Perform setup actions for jet handling at each event
 *
 *
 * @author P.A. Delsart
 */
class ATLAS_NOT_THREAD_SAFE JetGlobalEventSetup : public AthAlgorithm,
                                                  virtual public IIncidentListener {
public:
  JetGlobalEventSetup(const std::string& name, ISvcLocator* pSvcLocator);
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext&) override {return StatusCode::SUCCESS;};
  
  /// Implementation of IIncidentListener::handle
  virtual void handle(const Incident&) override;
};

#endif
