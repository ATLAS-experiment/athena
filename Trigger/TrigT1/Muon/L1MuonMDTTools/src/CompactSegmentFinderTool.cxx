/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * Compact Segment Finder algorithm overview
 *
 * This tool reconstructs an MDT segment in the global (z, R) plane starting
 * from a seed line provided by RPC information:
 *
 *   R = m * z + b
 *
 * Main steps:
 *
 *  1. Build per-hit information from the input MDT drift circles:
 *     - transform local hit positions to global coordinates
 *     - compute the drift radius
 *     - compute the two possible temporary intercepts b+ and b-
 *       corresponding to the left/right ambiguity of the drift tube
 *
 *  2. Split hits by multilayer and cluster them independently:
 *     - each hit can be associated to an existing cluster using either b+
 *       or b- if it is close enough to the cluster reference intercept
 *     - if no compatible cluster exists, create two new clusters seeded by
 *       b+ and b-
 *
 *  3. For each multilayer, select the best cluster:
 *     - first maximize the number of associated hits
 *     - then, in case of ties, choose the cluster closest to the RPC seed
 *       intercept rpcB
 *
 *  4. Resolve the left/right ambiguity for the hits belonging to the selected
 *     clusters and build corrected track points in the (z, R) plane
 *
 *  5. Fit the corrected points with a straight line:
 *       R = m * z + b
 *     and compute a simple chi2-like quality quantity
 *
 */

#include "CompactSegmentFinderTool.h"
#include "L1MuonMDTTools/L0MDTSegment.h"

namespace L0MDT {

  StatusCode CompactSegmentFinderTool::initialize() {
    // Retrieve the MDT identifier helper service
    ATH_CHECK(m_idHelperSvc.retrieve());
    return StatusCode::SUCCESS;
  }


  StatusCode CompactSegmentFinderTool::findSegments(
      const std::vector<const xAOD::MdtDriftCircle*>& driftCircles,
      const ActsTrk::GeometryContext& gctx,
      float m,
      float rpcB,
      std::vector<L0MDT::Segment>& segments) const {

    // Reset the output container for this call
    segments.clear();

    ATH_MSG_DEBUG("In CompactSegmentFinderTool::findSegments()");
    ATH_MSG_DEBUG("Size of drift circles vector: " << driftCircles.size()
                  << ", size of segments vector: " << segments.size());

    // Build a compact per-hit representation containing all quantities
    // needed for clustering and final fitting
    std::vector<HitInfo> hitInfos;
    ATH_CHECK(buildHitInfo(driftCircles, gctx, m, hitInfos));

    // A straight-line fit requires at least two usable hits
    if (hitInfos.size() < 2) {
      ATH_MSG_DEBUG("Not enough hits for clustering");
      return StatusCode::SUCCESS;
    }

    // Split hit indices by multilayer.
    // The current algorithm clusters each multilayer independently.
    std::array<std::vector<std::size_t>, 2> hitsPerML;
    for (std::size_t i = 0; i < hitInfos.size(); ++i) {
      hitsPerML[hitInfos[i].multilayer].push_back(i);
    }

    // Sort hits by increasing drift radius.
    // This follows the standalone logic and gives a deterministic cluster growth order.
    for (auto& indices : hitsPerML) {
      std::sort(indices.begin(), indices.end(),
                [&hitInfos](std::size_t a, std::size_t b) {
                  return hitInfos[a].driftRadius < hitInfos[b].driftRadius;
                });
    }

    // Store all clusters per multilayer, and the best cluster eventually selected
    std::array<std::vector<Cluster>, 2> clustersPerML;
    std::array<std::optional<Cluster>, 2> bestClusters;

    // Build clusters and choose the best one in each multilayer
    for (int ml = 0; ml < 2; ++ml) {
      clusterHits(hitInfos, hitsPerML[ml], clustersPerML[ml]);

      if (m_debugClusters) {
        ATH_MSG_INFO("ML " << ml << " has " << clustersPerML[ml].size() << " clusters");
        for (const Cluster& cl : clustersPerML[ml]) {
          ATH_MSG_INFO("  refB=" << cl.refB << " nHits=" << cl.hitIndices.size());
        }
      }

      bestClusters[ml] = chooseBestCluster(clustersPerML[ml], rpcB);
    }

    // Build corrected hit positions from the selected clusters.
    // The correction moves the tube center to the estimated track point using
    // the left/right solution selected during clustering.
    std::vector<float> correctedZ;
    std::vector<float> correctedR;
    correctedZ.reserve(hitInfos.size());
    correctedR.reserve(hitInfos.size());

    for (int ml = 0; ml < 2; ++ml) {
      if (!bestClusters[ml]) continue;

      const Cluster& cl = *bestClusters[ml];
      for (std::size_t i = 0; i < cl.hitIndices.size(); ++i) {
        const HitInfo& h = hitInfos[cl.hitIndices[i]];
        const bool plus = cl.plusFlags[i];

        // If the selected branch is b+, move to the corresponding side of the tube.
        // Otherwise use the opposite correction.
        if (plus) {
          correctedZ.push_back(h.z - h.deltaZ);
          correctedR.push_back(h.r + h.deltaR);
        } else {
          correctedZ.push_back(h.z + h.deltaZ);
          correctedR.push_back(h.r - h.deltaR);
        }
      }
    }

    // Require at least two corrected points for the final segment fit
    if (correctedZ.size() < 2) {
      ATH_MSG_DEBUG("Not enough corrected hits after cluster selection");
      return StatusCode::SUCCESS;
    }

    // Perform the final straight-line fit in the (z, R) plane
    const FitResult fit = fitLine(correctedZ, correctedR);
    if (!fit.valid) {
      ATH_MSG_DEBUG("Final fit failed");
      return StatusCode::SUCCESS;
    }


    float segZMin = *std::min_element(correctedZ.begin(), correctedZ.end());
    float segZMax = *std::max_element(correctedZ.begin(), correctedZ.end());
    float segZRef = 0.f;
    for (float z : correctedZ) {
      segZRef += z;
    }
    segZRef /= static_cast<float>(correctedZ.size());
    
    // Put the representative point on the fitted line
    float segRRef = fit.m * segZRef + fit.b;

    // Fill the output segment object here according to the actual EDM API
  
    L0MDT::Segment seg;
    seg.setM(fit.m);
    seg.setB(fit.b);
    seg.setChi2(fit.chi2);
    seg.setNHits(correctedZ.size());

    seg.setZMin(segZMin);
    seg.setZMax(segZMax);
    seg.setZRef(segZRef);
    seg.setRRef(segRRef);

    segments.push_back(seg);


    // Print the final CSF segment parameters in a parser-friendly format
    ATH_MSG_DEBUG("CSF segment fit | "
               << "m=" << fit.m
               << " b=" << fit.b
               << " chi2=" << fit.chi2
               << " nHits=" << correctedZ.size()
               << " zMin=" << segZMin
               << " zMax=" << segZMax
               << " zRef=" << segZRef
               << " rRef=" << segRRef);
    return StatusCode::SUCCESS;
  }

  ///////////////////////////////////////////////////////////////////////////////

  StatusCode CompactSegmentFinderTool::buildHitInfo(
      const std::vector<const xAOD::MdtDriftCircle*>& driftCircles,
      const ActsTrk::GeometryContext& gctx,
      float m,
      std::vector<HitInfo>& hitInfos) const {

    // Reset and pre-allocate the output hit container
    hitInfos.clear();
    hitInfos.reserve(driftCircles.size());

    // Common normalization factor used in the b+/b- construction and in the
    // geometrical correction from tube center to track point
    const float slopeNormalization = std::sqrt(1.f + m * m);

 

 
    for (const xAOD::MdtDriftCircle* dc : driftCircles) {

      const Identifier id = dc->identify();
      const auto& idh = m_idHelperSvc->mdtIdHelper();

      // Convert multilayer numbering from {1,2} to {0,1}
      const int ml = idh.multilayer(id) - 1;

      const MuonGMR4::MdtReadoutElement* detEl = dc->readoutElement();

      // Build the global position of the hit from the local measurement
      const Amg::Vector3D gpos= detEl->globalTubePos(gctx, dc->measurementHash()); 

      const float zHit = gpos.z();
      const float rHit = gpos.perp();
        

      // Drift radius is the distance between the wire and the track.
      // The sign is not used here because the left/right ambiguity is treated
      // explicitly through the two branches b+ and b-.
      const float driftRadius = dc->driftRadius();

      HitInfo info;
      info.dc = dc;
      info.z = zHit;
      info.r = rHit;
      info.driftRadius = driftRadius;
      info.multilayer = ml;

      // Temporary intercepts associated with the two possible sides of the tube
      info.bPlus  = (rHit - m * zHit) + slopeNormalization * driftRadius;
      info.bMinus = (rHit - m * zHit) - slopeNormalization * driftRadius;

      // Geometrical correction from tube center to track point
      info.deltaZ = (m / slopeNormalization) * driftRadius;
      info.deltaR = (1.f / slopeNormalization) * driftRadius;

      hitInfos.push_back(info);
    }

    return StatusCode::SUCCESS;
  }


  void CompactSegmentFinderTool::clusterHits(const std::vector<HitInfo>& hitInfos,
                                             const std::vector<std::size_t>& orderedIndices,
                                             std::vector<Cluster>& clusters) const {
    clusters.clear();


    for (std::size_t idx : orderedIndices) {
      const HitInfo& hit = hitInfos[idx];
      bool assigned = false;

      // Try to attach the hit to one of the existing clusters.
      // The hit can enter through either its b+ or b- branch.
      for (Cluster& cl : clusters) {
        const float dPlus  = std::abs(hit.bPlus  - cl.refB);
        const float dMinus = std::abs(hit.bMinus - cl.refB);

        // Prefer the b+ branch if it is both closer than b- and within tolerance
        if (dPlus < dMinus && dPlus < m_clusterTolerance) {
          cl.hitIndices.push_back(idx);
          cl.plusFlags.push_back(true);
          assigned = true;
          break;
        }

        // Otherwise try the b- branch if it is within tolerance
        else if (dMinus < m_clusterTolerance) {
          cl.hitIndices.push_back(idx);
          cl.plusFlags.push_back(false);
          assigned = true;
          break;
        }
      }

      // If the hit does not match any existing cluster, spawn two new clusters:
      // one for b+ and one for b-.
      if (!assigned && static_cast<int>(clusters.size()) + 2 <= m_maxClusters) {
        Cluster plusCluster;
        plusCluster.refB = hit.bPlus;
        plusCluster.hitIndices.push_back(idx);
        plusCluster.plusFlags.push_back(true);
        clusters.push_back(std::move(plusCluster));

        Cluster minusCluster;
        minusCluster.refB = hit.bMinus;
        minusCluster.hitIndices.push_back(idx);
        minusCluster.plusFlags.push_back(false);
        clusters.push_back(std::move(minusCluster));
      }
    }
  }


  std::optional<CompactSegmentFinderTool::Cluster>
  CompactSegmentFinderTool::chooseBestCluster(const std::vector<Cluster>& clusters,
                                              float rpcB) const {
    // No clusters available
    if (clusters.empty()) return std::nullopt;

    // First selection criterion: maximize the number of hits in the cluster
    std::size_t maxSize = 0;
    for (const Cluster& cl : clusters) {
      maxSize = std::max(maxSize, cl.hitIndices.size());
    }

    // Keep only clusters with maximal multiplicity
    std::vector<const Cluster*> candidates;
    for (const Cluster& cl : clusters) {
      if (cl.hitIndices.size() == maxSize) {
        candidates.push_back(&cl);
      }
    }

    // Tie-break criterion: choose the cluster closest to the RPC seed intercept
    auto it = std::min_element(
        candidates.begin(), candidates.end(),
        [rpcB](const Cluster* cl_a, const Cluster* cl_b) {
          return std::abs(cl_a->refB - rpcB) < std::abs(cl_b->refB - rpcB);
        });

    return **it;
  }


  CompactSegmentFinderTool::FitResult
  CompactSegmentFinderTool::fitLine(const std::vector<float>& zVals,
                                    const std::vector<float>& rVals,
                                    float sigma) const {
    FitResult out;

    const std::size_t n = zVals.size();

    // The fit is only meaningful if both vectors are consistent and contain
    // at least two points
    if (n < 2 || rVals.size() != n) return out;

    float sumZ = 0.f;
    float sumR = 0.f;
    float sumZZ = 0.f;
    float sumZR = 0.f;

    // Accumulate the standard least-squares sums for R = m*z + b
    for (std::size_t i = 0; i < n; ++i) {
      sumZ  += zVals[i];
      sumR  += rVals[i];
      sumZZ += zVals[i] * zVals[i];
      sumZR += zVals[i] * rVals[i];
    }

    const float N = static_cast<float>(n);
    const float den = N * sumZZ - sumZ * sumZ;

    // Degenerate case: all points have effectively the same z
    if (std::abs(den) < 1e-6f) return out;

    // Standard straight-line least-squares solution
    out.m = (N * sumZR - sumZ * sumR) / den;
    out.b = (sumR - out.m * sumZ) / N;

    // Compute a simple chi2-like quantity using a common sigma
    float chi2 = 0.f;
    for (std::size_t i = 0; i < n; ++i) {
      const float res = (rVals[i] - (out.m * zVals[i] + out.b)) / sigma;
      chi2 += res * res;
    }

    out.chi2 = chi2;
    out.valid = true;
    return out;
  }

} // end of namespace


