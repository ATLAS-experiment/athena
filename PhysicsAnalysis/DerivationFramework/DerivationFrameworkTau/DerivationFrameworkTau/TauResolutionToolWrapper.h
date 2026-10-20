/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TauResolutionToolWrapper.h
///////////////////////////////////////////////////////////////////

#ifndef DERIVATIONFRAMEWORK_TAURESOLUTIONTOOLWRAPPER_H
#define DERIVATIONFRAMEWORK_TAURESOLUTIONTOOLWRAPPER_H

#include <string>

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "TauAnalysisTools/TauResolutionTool.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "xAODTau/TauJetContainer.h"
#include <AsgDataHandles/WriteDecorHandle.h>

/**
 * wrapper tool for tau energy resolution
*/

namespace DerivationFramework {

  class TauResolutionToolWrapper : public extends<AthAlgTool, IAugmentationTool> {
    public:

    using base_class::base_class;

      virtual StatusCode initialize() override final;
      virtual StatusCode addBranches(const EventContext& ctx) const override final;

    private:
      SG::ReadHandleKey<xAOD::TauJetContainer> m_tauKey
         {this, "TauContainerName", "TauJets", "ReadHandleKey for input TauJetContainer"};

      ToolHandle < TauAnalysisTools::TauResolutionTool > m_tTauResolutionTool{this, "TauResolutionTool", "TauResolutionTool"};

      SG::WriteDecorHandleKey<xAOD::TauJetContainer> m_tesCompatibilityKey
         {this, "TESCompatibilityKey", m_tauKey, "TESCompatibility", "WriteDecorHandleKey for TESCompatibility decoration"};

  };
}

#endif
