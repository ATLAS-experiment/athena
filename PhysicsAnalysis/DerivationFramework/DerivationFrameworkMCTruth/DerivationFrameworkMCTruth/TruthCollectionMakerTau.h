/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_TRUTHCOLLECTIONMAKERTAU_H
#define DERIVATIONFRAMEWORK_TRUTHCOLLECTIONMAKERTAU_H

// Interface classes
#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"

// For the tool handle
#include "GaudiKernel/ToolHandle.h"

#include "TauAnalysisTools/IBuildTruthTaus.h"

namespace DerivationFramework {

  class TruthCollectionMakerTau : public extends<AthAlgTool, IAugmentationTool> {
  public:

    using base_class::base_class;

    virtual StatusCode initialize() override final;
    virtual StatusCode addBranches(const EventContext& ctx) const override final;

  private:

    PublicToolHandle<TauAnalysisTools::IBuildTruthTaus> m_buildTruthTaus{this, "BuildTruthTaus", "TauAnalysisTools::BuildTruthTaus/BuildTruthTaus"};

  };
}

#endif // DERIVATIONFRAMEWORK_TRUTHCOLLECTIONMAKERTAU_H
