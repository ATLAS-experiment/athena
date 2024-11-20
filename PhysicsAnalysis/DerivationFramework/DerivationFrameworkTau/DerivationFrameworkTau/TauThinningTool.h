/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORKTAU_TAUTHINNINGTOOL_H
#define DERIVATIONFRAMEWORKTAU_TAUTHINNINGTOOL_H

#include <atomic>

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IThinningTool.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODTau/TauTrackContainer.h"
#include "xAODPFlow/PFOContainer.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/VertexContainer.h"

#include "StoreGate/ThinningHandleKey.h"

#include "ExpressionEvaluation/ExpressionParserUser.h"

namespace DerivationFramework {

    class TauThinningTool : public extends<ExpressionParserUser<AthAlgTool>, IThinningTool> {
    public: 
      TauThinningTool(const std::string& t, const std::string& n, const IInterface* p);
      virtual ~TauThinningTool() = default;
      virtual StatusCode initialize() override;
      virtual StatusCode finalize() override;
      virtual StatusCode doThinning() const override;

    private:
      mutable std::atomic<unsigned int> m_ntot = 0;
      mutable std::atomic<unsigned int> m_npass = 0;
      StringProperty m_streamName { this, "StreamName", "", "Name of the stream being thinned" };
      Gaudi::Property<std::string> m_selectionString { this, "SelectionString", "", "" };
      SG::ThinningHandleKey<xAOD::TauJetContainer> m_taus { this, "Taus", "TauJets", "" };
      SG::ThinningHandleKey<xAOD::TauTrackContainer> m_tauTracks { this, "TauTracks", "TauTracks", "" };
      SG::ThinningHandleKey<xAOD::TrackParticleContainer> m_trackParticles { this, "TrackParticles", "InDetTrackParticles", "" };
      SG::ThinningHandleKey<xAOD::PFOContainer> m_neutralPFOs { this, "TauNeutralPFOs", "TauNeutralParticleFlowObjects", "" };
      SG::ThinningHandleKey<xAOD::VertexContainer> m_secondaryVertices { this, "TauSecondaryVertices", "TauSecondaryVertices", "" };
  };
}

#endif // DERIVATIONFRAMEWORKTAU_TAUTHINNINGTOOL_H
