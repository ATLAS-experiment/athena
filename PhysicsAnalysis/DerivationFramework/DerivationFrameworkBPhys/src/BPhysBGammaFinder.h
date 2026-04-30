/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
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

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "TrkEventPrimitives/ParticleHypothesis.h" //ParticleMasses struct

#include "InDetConversionFinderTools/VertexPointEstimator.h"
#include "xAODBPhys/BPhysHelper.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadDecorHandleKeyArray.h"

#include "TLorentzVector.h"

namespace Trk
{
    class V0Tools;
    class IVertexFitter;
    class TrkVKalVrtFitter;
}

namespace InDet
{
    class VertexPointEstimator;
    class TrackPairsSelector;
}

namespace DerivationFramework {
	
class BPhysBGammaFinder : public extends<AthAlgTool, IAugmentationTool> {

    public:

        BPhysBGammaFinder(const std::string& t, const std::string& n, const IInterface* p);

        StatusCode initialize() override;
        StatusCode finalize() override;

        virtual StatusCode addBranches(const EventContext& ctx) const override;
        TVector3 trackMomentum(const xAOD::Vertex & vxCandidate, int trkIndex) const;

    private:

        SG::ReadHandleKeyArray<xAOD::VertexContainer> m_BVertexCollectionsToCheck;
        SG::ReadDecorHandleKeyArray<xAOD::VertexContainer> m_passFlagsToCheck;

        ToolHandle <Trk::V0Tools> m_v0Tools;
        ToolHandle <Trk::IVertexFitter> m_vertexFitter;
        ToolHandle <InDet::VertexPointEstimator> m_vertexEstimator;

        SG::ReadHandleKey<xAOD::TrackParticleContainer> m_inputTrackParticleContainerName;
        SG::ReadHandleKey<xAOD::TrackParticleContainer> m_inputLowPtTrackContainerName;
        SG::WriteHandleKey<xAOD::VertexContainer> m_conversionContainerName;

        float m_maxDeltaQ;
        float m_Chi2Cut;
        float m_maxGammaMass;
        

  };
}

#endif // DERIVATIONFRAMEWORK_BPhysBGammaFinder_H
