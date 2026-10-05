/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
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

#include <string>
#include <unordered_map>
#include <vector>

class TH1;

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
    virtual StatusCode execute(const EventContext& ctx) override;

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

    /// cached pointers to the histograms of one reco vertex category
    struct RecoVertexHists {
      TH1* x{}; TH1* y{}; TH1* z{}; TH1* Lxy{}; TH1* pT{}; TH1* eta{}; TH1* phi{};
      TH1* mass{}; TH1* mu{}; TH1* chi2{}; TH1* dir{}; TH1* charge{}; TH1* H{}; TH1* HT{};
      TH1* minOpAng{}; TH1* maxOpAng{}; TH1* maxdR{}; TH1* mind0{}; TH1* maxd0{}; TH1* ntrk{};
      TH1* Trk_qOverP{}; TH1* Trk_theta{}; TH1* Trk_E{}; TH1* Trk_M{}; TH1* Trk_Pt{};
      TH1* Trk_Px{}; TH1* Trk_Py{}; TH1* Trk_Pz{}; TH1* Trk_Eta{}; TH1* Trk_Phi{};
      TH1* Trk_D0{}; TH1* Trk_Z0{}; TH1* Trk_errD0{}; TH1* Trk_errZ0{}; TH1* Trk_Chi2{};
      TH1* Trk_nDoF{}; TH1* Trk_charge{};
      // truth matching, only booked for matched categories (nullptr otherwise)
      TH1* positionRes_R{}; TH1* positionRes_Z{}; TH1* matchScore_weight{};
      TH1* matchScore_pt{};
    };
    std::unordered_map<std::string, RecoVertexHists> m_recoHists;

    void fillRecoHistograms(const xAOD::Vertex* secVtx, const std::vector<const RecoVertexHists*>& categories);
    void fillTruthHistograms(const xAOD::TruthVertex* truthVtx, const std::string& truthType);
  };
} // namespace CP
#endif
