/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef L1MuonMDTTools_COMPACTSEGMENTFINDERTOOL_H
#define L1MuonMDTTools_COMPACTSEGMENTFINDERTOOL_H
#include "AthenaBaseComps/AthAlgTool.h"

// local includes
#include "xAODMuonPrepData/MdtDriftCircleContainer.h"
#include "L1MuonMDTTools/IL0MDTSegmentFinderTool.h"
#include "L1MuonMDTTools/L0MDTSegment.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "ActsGeometryInterfaces/GeometryContext.h"
#include "MuonReadoutGeometryR4/MdtReadoutElement.h"
#include <optional>

// namespace for the L0MDTS related classes
namespace L0MDT {

/**
 * @class L0MDTSegmentFinderTool
 * @brief Athena tool to reconstruct L0MDT segments.
 *
 * This tool implements a compact MDT segment finding algorithm in the global
 * (z, R) plane. Starting from a seed line provided externally, it:
 *
 *  - builds a compact per-hit representation from the input MDT drift circles
 *  - resolves the left/right ambiguity through temporary intercept clustering
 *  - selects the best cluster in each multilayer
 *  - builds corrected track points
 *  - fits the final segment parameters
 */
class CompactSegmentFinderTool : public extends<AthAlgTool, IL0MDTSegmentFinderTool> {

public:
  using base_class::base_class;
  virtual ~CompactSegmentFinderTool() override = default;

  /// Standard Athena initialize method
  virtual StatusCode initialize() override;

  /**
   * @brief Main segment finding entry point
   *
   * @param driftCircles Input MDT drift circles already selected upstream
   * @param gctx Geometry context used to transform local hit positions to global coordinates
   * @param m Slope of the seed line in the (z, R) plane
   * @param b Intercept of the seed line in the (z, R) plane
   * @param segments Output container for reconstructed segments
   */
  virtual StatusCode findSegments(const std::vector<const xAOD::MdtDriftCircle*>& driftCircles,
                                  const ActsTrk::GeometryContext& gctx, float m, float b,
                                  std::vector<L0MDT::Segment>& segments) const override;

private:

  /// Maximum allowed distance in intercept space when attaching a hit to an existing cluster
  Gaudi::Property<float> m_clusterTolerance{this, "ClusterTolerance", 5.f,
      "Maximum allowed distance in intercept space when attaching a hit to an existing cluster"};

  /// Maximum number of temporary clusters allowed during clustering
  Gaudi::Property<int> m_maxClusters{this, "MaxClusters", 6,
      "Maximum number of temporary clusters allowed during clustering"};

  /// Debug flag for printing cluster content and selection
  Gaudi::Property<bool> m_debugClusters{this, "DebugClusters", false,
      "Enable debug printout of cluster content and selection"};

  /**
   * @struct HitInfo
   * @brief Compact representation of one MDT hit used by the clustering and fit steps
   *
   * Members include:
   *  - global tube-center coordinates (z, r)
   *  - drift radius
   *  - the two temporary intercepts bPlus and bMinus associated with the
   *    left/right ambiguity
   *  - the geometrical corrections deltaZ and deltaR used to build corrected
   *    track points once a branch is selected
   *  - the multilayer index
   */
  struct HitInfo {
    const xAOD::MdtDriftCircle* dc{nullptr};
    float z{0.f};
    float r{0.f};
    float driftRadius{0.f};
    float bPlus{0.f};
    float bMinus{0.f};
    float deltaZ{0.f};
    float deltaR{0.f};
    int multilayer{0};
  };

  /**
   * @struct Cluster
   * @brief Temporary cluster of hit intercept hypotheses
   *
   * refB is the reference intercept associated with the cluster.
   * hitIndices stores the indices of the hits belonging to the cluster.
   * plusFlags records whether each hit entered through the bPlus or bMinus branch.
   */
  struct Cluster {
    float refB{0.f};
    std::vector<std::size_t> hitIndices;
    std::vector<bool> plusFlags;
  };

  /**
   * @struct FitResult
   * @brief Output of the final straight-line fit
   *
   * m and b are the fitted line parameters in the (z, R) plane,
   * chi2 is a simple chi2-like quality estimator,
   * and valid indicates whether the fit succeeded.
   */
  struct FitResult {
    float m{0.f};
    float b{0.f};
    float chi2{0.f};
    bool valid{false};
  };

  /**
   * @brief Build the compact per-hit representation used by the algorithm
   *
   * For each drift circle, this method computes:
   *  - global hit position in the (z, R) plane
   *  - drift radius
   *  - temporary intercepts bPlus and bMinus
   *  - correction terms deltaZ and deltaR
   */
  virtual StatusCode buildHitInfo(const std::vector<const xAOD::MdtDriftCircle*>& driftCircles,
                                  const ActsTrk::GeometryContext& gctx, float m,
                                  std::vector<HitInfo>& hitInfos) const;

  /**
   * @brief Cluster ordered hits in intercept space
   *
   * Hits are attached to existing clusters using the closest compatible branch
   * (bPlus or bMinus). If no compatible cluster exists, two new clusters are
   * created from the two possible intercept hypotheses.
   */
  void clusterHits(const std::vector<HitInfo>& hitInfos,
                   const std::vector<std::size_t>& orderedIndices,
                   std::vector<Cluster>& clusters) const;

  /// MDT identifier helper service
  ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{
      this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

  /**
   * @brief Select the best cluster among the available candidates
   *
   * The current policy is:
   *  1. maximize the number of hits in the cluster
   *  2. in case of ties, choose the cluster closest to the input seed intercept b
   */
  std::optional<Cluster> chooseBestCluster(const std::vector<Cluster>& clusters, float b) const;

  /**
   * @brief Fit a straight line in the (z, R) plane
   *
   * The fitted model is:
   *   R = m * z + b
   *
   * @param zVals Input z coordinates
   * @param rVals Input R coordinates
   * @param sigma Common uncertainty used in the chi2 computation
   */
  FitResult fitLine(const std::vector<float>& zVals,
                    const std::vector<float>& rVals,
                    float sigma = 1.f / 8.f) const;
};

} // end of namespace
#endif