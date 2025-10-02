/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRACKINGANALYSISALGORITHMS_SECVERTEXTRUTHMATCHALG_H
#define TRACKINGANALYSISALGORITHMS_SECVERTEXTRUTHMATCHALG_H

// Gaudi/Athena include(s):
#include "AnaAlgorithm/AnaAlgorithm.h"
#include "AsgTools/ToolHandle.h"

// SG includes
#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTruth/TruthVertexContainer.h"

// Local include(s):
#include "InDetSecVtxTruthMatchTool/IInDetSecVtxTruthMatchTool.h"

// Asg includes
#include <AsgTools/PropertyWrapper.h>
#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgDataHandles/ReadHandle.h>

namespace CP {

   /// Algorithm to perform truth matching on secondary vertices
   ///
   /// @author Jackson Burzynski <jackson.carl.burzynski@cern.ch>
   ///
  class SecVertexTruthMatchAlg final : public EL::AnaAlgorithm {

  public:
    /// Regular Algorithm constructor
    SecVertexTruthMatchAlg( const std::string& name, ISvcLocator* svcLoc );
    virtual StatusCode initialize() override;
    virtual StatusCode execute() override;

  private:
    // Input (reco) Secondary Vertices
    SG::ReadHandleKey<xAOD::VertexContainer> m_secVtxContainerKey{this, "SecondaryVertexContainer", "VrtSecInclusive_SecondaryVertices",
                                                                "Secondary vertex container"};
    // Input Truth Vertices
    SG::ReadHandleKey<xAOD::TruthVertexContainer> m_truthVtxContainerKey{this, "TruthVertexContainer", "TruthVertices",
                                                                        "Truth vertex container"};
    // Input Tracks
    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_trackParticleContainerKey{this, "TrackParticleContainer", "InDetTrackParticles",
                                                                "Track container"};

    Gaudi::Property<std::vector<int>> m_targetPDGIDs{this, "TargetPDGIDs", {}, "List of PDGIDs to select for matching"};

    Gaudi::Property<bool> m_writeHistograms{this, "WriteHistograms", true, "Write histograms"};
    
    ToolHandle<IInDetSecVtxTruthMatchTool> m_matchTool{this, "MatchTool", "InDetSecVtxTruthMatchTool"};
    Gaudi::Property<bool> m_doMuSA{this, "doMuSA", false, "MuSA mode for wider histogram ranges"};
    Gaudi::Property<bool> m_doSMOrigin{this, "doSMOrigin", false, "Enable SM origin categorization"};

    void fillRecoHistograms(const xAOD::Vertex* secVtx, const std::string& matchType);
    void fillTruthHistograms(const xAOD::TruthVertex* truthVtx, const std::string& truthType);
    void fillOriginHistograms(const xAOD::Vertex* secVtx, const std::string& originType);
  };
} // namespace CP
#endif
