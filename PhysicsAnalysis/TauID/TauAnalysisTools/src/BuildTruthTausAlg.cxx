/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "BuildTruthTausAlg.h"
#include "GaudiKernel/EventContext.h"

namespace TauAnalysisTools {

  BuildTruthTausAlg::BuildTruthTausAlg(const std::string& name, ISvcLocator* svcLoc)
    : AthReentrantAlgorithm(name, svcLoc)
  {}

  StatusCode BuildTruthTausAlg::initialize()
  {
    ATH_CHECK(m_buildTruthTaus.retrieve());
    return StatusCode::SUCCESS;
  }

  StatusCode BuildTruthTausAlg::execute(const EventContext& ctx) const
  {
    TauAnalysisTools::BuildTruthTaus::TruthTausEvent taus;

    ATH_CHECK(m_buildTruthTaus->retrieveTruthTaus(taus, ctx));

    return StatusCode::SUCCESS;
  }

}
