/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Tool to decorate the Electrons object with additional information for merged electron ID
// Authors: A.Morley

#ifndef DerivationFrameworkHiggs_MergedElectronDetailsDecorator_H
#define DerivationFrameworkHiggs_MergedElectronDetailsDecorator_H

#include <string>
#include <vector>
#include <TEnv.h>
#include <TString.h>
#include <TSystem.h>

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "TrkVertexAnalysisUtils/V0Tools.h"
#include "TrkVertexFitterInterfaces/IVertexFitter.h"
#include "egammaInterfaces/IEMExtrapolationTools.h"
#include "CaloDetDescr/CaloDetDescrManager.h"

#include "xAODTracking/TrackParticleContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODEventInfo/EventInfo.h"

namespace DerivationFramework {

  class MergedElectronDetailsDecorator : public extends<AthAlgTool, IAugmentationTool> {

  public:

    using base_class::base_class;

    virtual StatusCode initialize() override final;
    virtual StatusCode addBranches(const EventContext& ctx) const override final;

  private:

    void fillMatchDetails( std::vector<float>& trkMatchTrk, const  xAOD::TrackParticle* tp, const xAOD::CaloCluster* cluster) const;
    static int  nSiHits( const xAOD::TrackParticle * tp ) ;
    void fillTrackDetails(const xAOD::Electron* el, bool isMC) const;
    void fillVertexDetails(const xAOD::Electron* el) const;
    static void fillClusterDetails(const xAOD::Electron* el) ;
    void fillTruthDetails( std::vector<float>& trkMatchTrk, const xAOD::TrackParticle* tp, const xAOD::CaloCluster* cluster) const;

    PublicToolHandle<IEMExtrapolationTools> m_emExtrapolationTool{this, "EMExtrapolationTool", "EMExtrapolationTools"};
    PublicToolHandle<Trk::IVertexFitter> m_VertexFitter{this, "VertexFitterTool", "Trk::TrkVkalVrtFitter"};
    PublicToolHandle<Trk::V0Tools> m_V0Tools{this, "V0Tools", "Trk::V0Tools"};

    Gaudi::Property<float> m_minET{this, "MinET", 5000.f};

    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{ this,
                                                       "EventInfoKey",
                                                       "EventInfo",
                                                       "" };
    SG::ReadHandleKey<xAOD::ElectronContainer> m_electronKey{ this,
                                                              "ElectronKey",
                                                              "Electrons",
                                                              "" };
    SG::ReadCondHandleKey<CaloDetDescrManager> m_caloMgrKey{
      this,
      "CaloDetDescrManager",
      "CaloDetDescrManager"
    };

  }; /// class

} /// namespace

#endif
