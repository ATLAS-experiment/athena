/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_DERIVATIONKERNEL_H
#define DERIVATIONFRAMEWORK_DERIVATIONKERNEL_H 1

#include <string>
#include <vector>

#include "AthenaBaseComps/AthAlgorithm.h"
#include "EventBookkeeperTools/FilterReporterParams.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "DerivationFrameworkInterfaces/ISkimmingTool.h"
#include "DerivationFrameworkInterfaces/IThinningTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "GaudiKernel/IChronoStatSvc.h"

namespace DerivationFramework {

  /////////////////////////////////////////////////////////////////////////////
  class DerivationKernel : public AthAlgorithm {

  public:
    using AthAlgorithm::AthAlgorithm;

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) override;
    virtual StatusCode finalize() override;
  private:
    // Tools etc
    PublicToolHandleArray<ISkimmingTool> m_skimmingTools{this, "SkimmingTools", {} };
    PublicToolHandleArray<IThinningTool> m_thinningTools{this, "ThinningTools", {} };
    PublicToolHandleArray<IAugmentationTool> m_augmentationTools{this, "AugmentationTools", {} };
    ServiceHandle<IChronoStatSvc> m_chronoSvc{this, "ChronoStatSvc",  "ChronoStatSvc"};

    Gaudi::Property<bool> m_runSkimmingFirst{this, "RunSkimmingFirst", false};
    Gaudi::Property<bool> m_doChronoStat{this,"doChronoStat",true,"use ChronoStatSvc (only in serial jobs)"};

    FilterReporterParams m_filterParams {this, "GoodRunsList", "Good Runs Lists selection"};

    // Some counters
    int m_eventCounter{};
    int m_acceptCntr{};

  };

} // end of namespace
#endif
