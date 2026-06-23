/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// UFOTrackParticleThinning.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef DERIVATIONFRAMEWORK_UFOTRACKPARTICLETHINNING_H
#define DERIVATIONFRAMEWORK_UFOTRACKPARTICLETHINNING_H

#include <string>
#include <vector>
#include <atomic>

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IThinningTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODJet/JetContainer.h"
#include "StoreGate/ThinningHandleKey.h"
#include "StoreGate/ReadHandleKey.h"

#include "xAODPFlow/FlowElementContainer.h"

#include "ExpressionEvaluation/ExpressionParserUser.h"

namespace DerivationFramework {

  class UFOTrackParticleThinning: public extends<ExpressionParserUser<AthAlgTool>, IThinningTool> {
  public: 
    UFOTrackParticleThinning(const std::string& t, const std::string& n, const IInterface* p);
    virtual ~UFOTrackParticleThinning();
    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;
    virtual StatusCode doThinning(const EventContext& ctx) const override;

  private:
    StringProperty m_streamName{ this, "StreamName", "", "Name of the stream being thinned" };
    SG::ThinningHandleKey<xAOD::TrackParticleContainer> m_inDetSGKey{ this, "InDetTrackParticlesKey", "InDetTrackParticles", "" };
    Gaudi::Property<bool> m_thinTracks{this,"ThinTrackingContainer",true,"Toggle thinning of container with name InDetTrackParticlesKey"};
    Gaudi::Property<std::string> m_PFOSGKey{ this, "PFOCollectionSGKey", "Global", "" };
    SG::ThinningHandleKey<xAOD::FlowElementContainer> m_PFOChargedSGKey{ this, "PFOChargedCollectionSGKey", "GlobalChargedParticleFlowObjects", "" };
    SG::ThinningHandleKey<xAOD::FlowElementContainer> m_PFONeutralSGKey{ this, "PFONeutralCollectionSGKey", "GlobalNeutralParticleFlowObjects", "" };
    Gaudi::Property<std::vector<std::string>> m_addPFOSGKey{ this, "AdditionalPFOKey", {}, ""};
    SG::ThinningHandleKey<xAOD::FlowElementContainer> m_tmpAddPFOChargedSGKey{ this, "TempAddPFOChargedKey","",""};
    SG::ThinningHandleKey<xAOD::FlowElementContainer> m_tmpAddPFONeutralSGKey{ this, "TempAddPFONeutralKey","",""};
    std::vector<SG::ThinningHandleKey<xAOD::FlowElementContainer>> m_addPFOChargedSGKey;
    std::vector<SG::ThinningHandleKey<xAOD::FlowElementContainer>> m_addPFONeutralSGKey;
    SG::ThinningHandleKey<xAOD::FlowElementContainer> m_ufoSGKey{ this, "UFOKey", "UFOCSSK", "" };
    SG::ReadHandleKey<xAOD::JetContainer> m_jetSGKey{ this, "JetKey", "", ""};
    StringProperty m_selectionString{ this, "SelectionString", "", "" };

  }; 
}

#endif // DERIVATIONFRAMEWORK_UFOTRACKPARTICLETHINNING_H
