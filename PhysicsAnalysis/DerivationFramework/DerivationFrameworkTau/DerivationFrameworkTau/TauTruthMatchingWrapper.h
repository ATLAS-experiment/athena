/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TauTruthMatchingWrapper.h
// Author: Evelina Bouhova-Thacker (e.bouhova@cern.ch)
///////////////////////////////////////////////////////////////////

#ifndef DERIVATIONFRAMEWORK_TAUTRUTHMATCHINGWRAPPER_H
#define DERIVATIONFRAMEWORK_TAUTRUTHMATCHINGWRAPPER_H

#include <string>

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "TauAnalysisTools/ITauTruthMatchingTool.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "xAODTau/TauJetContainer.h"

/**
 * wrapper tool for tau truth matching
*/

namespace DerivationFramework {

  class TauTruthMatchingWrapper : public extends<AthAlgTool, IAugmentationTool> {
    public:

    using base_class::base_class;

      virtual StatusCode initialize() override final;
      virtual StatusCode addBranches(const EventContext& ctx) const override final;

    private:
      SG::ReadHandleKey<xAOD::TauJetContainer> m_tauKey
         {this, "TauContainerName", "TauJets", "ReadHandleKey for input TauJetContainer"};

      ToolHandle < TauAnalysisTools::ITauTruthMatchingTool > m_tTauTruthMatchingTool{this, "TauTruthMatchingTool", "TauAnalysisTools::TauTruthMatchingTool"};

      SG::WriteDecorHandleKey<xAOD::TauJetContainer> m_isTruthMatchedKey
         {this, "IsTruthMatchedKey", m_tauKey, "IsTruthMatched", "WriteDecorHandleKey for IsTruthMatched decoration"};

      SG::WriteDecorHandleKey<xAOD::TauJetContainer> m_truthJetLinkKey
         {this, "TruthJetLinkKey", m_tauKey, "truthJetLink", "WriteDecorHandleKey for truthJetLink decoration"};

      SG::WriteDecorHandleKey<xAOD::TauJetContainer> m_truthParticleLinkKey
         {this, "TruthParticleLinkKey", m_tauKey, "truthParticleLink", "WriteDecorHandleKey for truthParticleLink decoration"};

      SG::WriteDecorHandleKey<xAOD::TauJetContainer> m_tauOriginKey
         {this, "TauOriginKey", m_tauKey, "tauOriginDevOnly", "WriteDecorHandleKey for tauOriginDev decoration"};

  };
}

#endif
