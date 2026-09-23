/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// ****************************************************************************
// ----------------------------------------------------------------------------
// BPhysBGammaFinder header file
//
// Tatiana Lyubushkina <tatiana.lyubushkina@cern.ch>

// ----------------------------------------------------------------------------
// ****************************************************************************
#ifndef DERIVATIONFRAMEWORK_BPHYSBGAMMAFINDER_H
#define DERIVATIONFRAMEWORK_BPHYSBGAMMAFINDER_H

#include <string>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "TrkEventPrimitives/ParticleHypothesis.h" //ParticleMasses struct

#include "TrkVertexAnalysisUtils/V0Tools.h"
#include "TrkVertexFitterInterfaces/IVertexFitter.h"
#include "TrkVKalVrtFitter/TrkVKalVrtFitter.h"
#include "InDetConversionFinderTools/VertexPointEstimator.h"
#include "xAODBPhys/BPhysHelper.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadDecorHandleKeyArray.h"

#include "TLorentzVector.h"

namespace InDet
{
  class TrackPairsSelector;
}

namespace DerivationFramework {

  class BPhysBGammaFinder : public AthReentrantAlgorithm {

  public:

    using AthReentrantAlgorithm::AthReentrantAlgorithm;

    StatusCode initialize() override;

    virtual StatusCode execute(const EventContext& ctx) const override;
    TVector3 trackMomentum(const xAOD::Vertex & vxCandidate, int trkIndex) const;

  private:
    // TODO Configurable property declarations in header
    SG::ReadHandleKeyArray<xAOD::VertexContainer> m_BVertexCollectionsToCheck{this, "BVertexContainers", {}};
    SG::ReadDecorHandleKeyArray<xAOD::VertexContainer> m_passFlagsToCheck{this, "PassFlagsToCheck", {}};

    PublicToolHandle <Trk::V0Tools> m_v0Tools{this, "V0Tools", "Trk::V0Tools"};
    ToolHandle <Trk::IVertexFitter> m_vertexFitter{this, "VertexFitterTool", "Trk::TrkVKalVrtFitter"};
    ToolHandle <InDet::VertexPointEstimator> m_vertexEstimator{this, "VertexEstimator", "InDet::VertexPointEstimator"};

    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_inputTrackParticleContainerName{this, "InputTrackParticleContainerName", "InDetTrackParticles"};
    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_inputLowPtTrackContainerName{this, "InputLowPtTrackContainerName", "LowPtRoITrackParticles"};
    SG::WriteHandleKey<xAOD::VertexContainer> m_conversionContainerName{this, "ConversionContainerName", "BPhysConversionCandidates"};

    Gaudi::Property<float> m_maxDeltaQ{this, "MaxDeltaQ", 700.0}; // Maximum mass difference between di-muon+conversion and di-muon};
    Gaudi::Property<float> m_Chi2Cut{this, "Chi2Cut", 20.0};
    Gaudi::Property<float> m_maxGammaMass{this, "MaxGammaMass", 100.0};


  };
}

#endif // DERIVATIONFRAMEWORK_BPhysBGammaFinder_H
