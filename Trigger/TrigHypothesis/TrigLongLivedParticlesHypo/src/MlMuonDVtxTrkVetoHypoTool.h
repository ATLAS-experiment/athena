/*
Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MLMUONDVTXTRKVETOHYPOTOOL_H
#define MLMUONDVTXTRKVETOHYPOTOOL_H

#include "Gaudi/Property.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "TrigCompositeUtils/HLTIdentifier.h"
#include "TrigCompositeUtils/TrigCompositeUtils.h"
#include "xAODTracking/TrackParticleContainer.h"

#include <vector>

/** @brief Hypothesis tool for the prompt-track veto of the Muon Machine
 *         Learning-based displaced vertex chain. The DV direction is the
 *         leading cluster of muon segments; the event is rejected if a
 *         prompt full-scan track from the hard-scatter vertex points to it. */

class MlMuonDVtxTrkVetoHypoTool : virtual public ::AthAlgTool {
  public:

    MlMuonDVtxTrkVetoHypoTool(const std::string& type,
                              const std::string& name,
                              const IInterface* parent);

    virtual ~MlMuonDVtxTrkVetoHypoTool() = default;

    /// Cluster of muon segments in eta-phi (axis = segment average)
    struct SegCluster {
        float eta {0.f};
        float phi {0.f};
        unsigned int nSeg {0};
    };

    struct Info {
        TrigCompositeUtils::Decision* outDecision {nullptr};
        const std::vector<SegCluster>* clusters {nullptr};  // sorted by nSeg, leading first
        const xAOD::TrackParticleContainer* tracks {nullptr};
        bool hasPV {false};
        float zPV {0.f};
        TrigCompositeUtils::DecisionIDContainer activeChainIDs {};
    };
    StatusCode decide(Info& info) const;

    /// Number of prompt tracks pointing to the cluster
    unsigned int nPromptTracks(const SegCluster& cl, const Info& info) const;

  private:

    HLT::Identifier m_decisionId;
    Gaudi::Property<float> m_minTrackPt{this, "MinTrackPt", 5000.f, "Minimum pT [MeV] of a vetoing track"};
    Gaudi::Property<float> m_maxTrackDR{this, "MaxTrackDR", 0.4f, "Maximum deltaR between a vetoing track and the DV direction"};
    Gaudi::Property<float> m_maxDz0SinTheta{this, "MaxDz0SinTheta", 2.f,
        "Maximum |(z0 - zPV) sin(theta)| [mm] of a vetoing track; < 0: no requirement"};
    Gaudi::Property<unsigned int> m_minClusterSegments{this, "MinClusterSegments", 2, "Minimum segments in a DV-direction cluster"};
    Gaudi::Property<bool> m_onlyLeadingCluster{this, "OnlyLeadingCluster", true,
        "Veto on the leading cluster only; otherwise accept if any cluster is free of prompt tracks"};
    Gaudi::Property<unsigned int> m_maxClusters{this, "MaxClusters", 5, "Leading clusters tried as DV direction (OnlyLeadingCluster = false)"};
    Gaudi::Property<bool> m_acceptAll{this, "AcceptAll", false, "Accept every event (no veto)"};
  };

#endif
