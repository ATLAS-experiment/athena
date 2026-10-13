Tracking Analysis Algorithms
============================

This package is meant to collect dual-use/analysis algorithms that are
related to tracking.

Current algorithm(s) in the package:
  - [CP::VertexSelectionAlg](TrackingAnalysisAlgorithms/VertexSelectionAlg.h):
    Filter algorithm requiring a certain number of primary vertices in
    the event.
  - [CP::InDetTrackSelectionAlg](TrackingAnalysisAlgorithms/InDetTrackSelectionAlg.h):
    Selects tracks using the InDetTrackSelectionTool and, optionally, the
    InDetTrackTruthFilterTool.
  - [CP::InDetTrackBiasingAlg](TrackingAnalysisAlgorithms/InDetTrackBiasingAlg.h):
    Applies the InDetTrackBiasingTool to tracks.
  - [CP::InDetTrackSmearingAlg](TrackingAnalysisAlgorithms/InDetTrackSmearingAlg.h):
    Applies the InDetTrackSmearingTool to tracks.
  - [CP::InDetTrackExtraVarDecoratorAlg](TrackingAnalysisAlgorithms/InDetTrackExtraVarDecoratorAlg.h):
    Decorates tracks with their momentum, eta and charge.
  - [CP::TrackParticleMergerAlg](TrackingAnalysisAlgorithms/TrackParticleMergerAlg.h):
    Merges several track particle containers into a single one.
  - [CP::SecVertexTruthMatchAlg](TrackingAnalysisAlgorithms/SecVertexTruthMatchAlg.h):
    Fills validation histograms for secondary vertices matched to truth
    vertices with the InDetSecVtxTruthMatchTool.
