/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_COMMONAUGMENTATION_H
#define DERIVATIONFRAMEWORK_COMMONAUGMENTATION_H 1

#include <string>
#include <vector>
#include <list>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "GaudiKernel/IChronoStatSvc.h"

namespace DerivationFramework {

  /////////////////////////////////////////////////////////////////////////////
  class CommonAugmentation : public AthReentrantAlgorithm {

  public:
    CommonAugmentation (const std::string& name, ISvcLocator* pSvcLocator);
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;
    virtual StatusCode finalize() override;
  private:
    // Tools etc
    PublicToolHandleArray<IAugmentationTool> m_augmentationTools{this, "AugmentationTools", {} };
    ServiceHandle<IChronoStatSvc>      m_chronoSvc{this, "ChronoStatSvc",  "ChronoStatSvc"};

  };

} // end of namespace
#endif
