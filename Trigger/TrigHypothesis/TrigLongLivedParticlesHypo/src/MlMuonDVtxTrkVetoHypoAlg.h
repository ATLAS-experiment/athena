/*
Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MLMUONDVTXTRKVETOHYPOALG_H
#define MLMUONDVTXTRKVETOHYPOALG_H

// Framework includes
#include "DecisionHandling/HypoBase.h"
#include "MlMuonDVtxTrkVetoHypoTool.h"
#include "xAODMuon/MuonSegmentContainer.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODTrigger/TrigCompositeContainer.h"

/** @brief Prompt-track veto for the Muon Machine Learning-based
 *         displaced vertex chain, run after the ML score cut.
 *         The muon segments are clustered in eta-phi to get the DV
 *         direction; the hypo tools reject the event if a prompt
 *         full-scan track from the hard-scatter vertex points to it. */
class MlMuonDVtxTrkVetoHypoAlg : public ::HypoBase  {
  public:
    using HypoBase::HypoBase;

    virtual ~MlMuonDVtxTrkVetoHypoAlg() = default;
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& context) const override;

private:

  /// Clusters of segments, sorted by number of segments (leading first)
  std::vector<MlMuonDVtxTrkVetoHypoTool::SegCluster> clusterSegments(const xAOD::MuonSegmentContainer& segments) const;

  /// DEBUG only: clusters and nearby tracks, for the offline cut scan
  void dumpEvent(const EventContext& ctx, const MlMuonDVtxTrkVetoHypoTool::Info& info, unsigned int nSegments) const;

  ToolHandleArray< MlMuonDVtxTrkVetoHypoTool > m_hypoTools {this, "HypoTools", {},"Tools that perform actual selection"};
  SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_segmentKey{this, "SegmentKey", "MsSegments", "Muon segments"};
  SG::ReadHandleKey<xAOD::TrackParticleContainer> m_trackKey{this, "TracksKey", "", "Full-scan tracks"};
  SG::ReadHandleKey<xAOD::VertexContainer> m_vertexKey{this, "VerticesKey", "", "Full-scan primary vertices"};
  SG::WriteHandleKey<xAOD::TrigCompositeContainer> m_outputKey{
      this, "DVTrkVetoOutputKey", "EF_MuonDVTrkVetoOutput", "xAOD::TrigComposite with the DV direction, linked as feature"};

  Gaudi::Property<float> m_clusterDR{this, "ClusterDR", 0.4f, "deltaR of the segment clustering"};
  Gaudi::Property<unsigned int> m_dumpMaxClusters{this, "DumpMaxClusters", 5, "DEBUG dump: leading clusters with their tracks"};
  Gaudi::Property<float> m_dumpMinTrackPt{this, "DumpMinTrackPt", 2000.f, "DEBUG dump: minimum track pT [MeV]"};
  Gaudi::Property<float> m_dumpMaxTrackDR{this, "DumpMaxTrackDR", 0.6f, "DEBUG dump: maximum track-cluster deltaR"};
};
#endif
