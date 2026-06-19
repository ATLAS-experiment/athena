/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>

#ifndef PARTONS_RUNPARTONHISTORYALG_H
#define PARTONS_RUNPARTONHISTORYALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>

#include "PartonHistory/CalcPartonHistory.h"

namespace CP {

class RunPartonHistoryAlg final : public EL::AnaAlgorithm {

 public:
  RunPartonHistoryAlg(const std::string& name, ISvcLocator* pSvcLocator);
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) override;

 private:
  std::string m_PartonScheme;
  std::unique_ptr<CalcPartonHistory> m_PartonHistory;
};

}  // namespace CP

#endif
