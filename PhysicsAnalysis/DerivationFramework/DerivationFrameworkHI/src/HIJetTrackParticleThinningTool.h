/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// HIJetTrackParticleThinningTool.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef DERIVATIONFRAMEWORK_HIJETTRACKPARTICLETHINNINGTOOL_H
#define DERIVATIONFRAMEWORK_HIJETTRACKPARTICLETHINNINGTOOL_H

// Gaudi & Athena basics
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODJet/JetContainer.h"
#include "StoreGate/ThinningHandleKey.h"

// DerivationFramework includes
#include "DerivationFrameworkInterfaces/IThinningTool.h"
#include "AsgTools/ToolHandle.h"
#include "InDetTrackSelectionTool/IInDetTrackSelectionTool.h"

#include <string>
#include <atomic>

namespace DerivationFramework {

  class HIJetTrackParticleThinningTool : public extends<AthAlgTool, IThinningTool> {
  
  public:
    // Constructor with parameters
    HIJetTrackParticleThinningTool( const std::string& t, const std::string& n, const IInterface* p);

    // Destructor
    virtual ~HIJetTrackParticleThinningTool();

    // Athena algtool's Hooks
    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;

    // Check current event passes filter
    virtual StatusCode doThinning(const EventContext& ctx) const override;

  private:
    StringProperty m_streamName
    { this, "StreamName", "", "Name of the stream being thinned" };

    SG::ReadHandleKey<xAOD::VertexContainer> m_vertexKey
    { this, "PrimaryVertexKey", "PrimaryVertices", "Primary vertex container"};
    SG::ReadHandleKey<xAOD::JetContainer> m_jetKey
    { this, "JetKey", "", "Jet collection container"};
    SG::ReadDecorHandleKey<xAOD::VertexContainer> m_sumPt2Key
    { this, "SumPt2Key", "PrimaryVertices.sumPt2", "SumPt2 decoration"};
    SG::ThinningHandleKey<xAOD::TrackParticleContainer> m_inDetSGKey
    { this, "InDetTrackParticlesKey", "InDetTrackParticles", "" };

    ToolHandle<InDet::IInDetTrackSelectionTool> m_trkSelTool
    { this, "TrackSelectionTool", "", "Track selection tool"};
    Gaudi::Property<std::string> m_vertexScheme
    { this, "PrimaryVertexSelection", "sumPt2", "Vertex selection scheme"};
  
    mutable std::atomic<unsigned int> m_ntot = 0;
    mutable std::atomic<unsigned int> m_npass = 0;
  };
}

#endif