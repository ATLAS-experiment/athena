/*
  Copyright (C) 2002-2025 CERN
  for the benefit of the ATLAS collaboration
*/
#ifndef MUONINFERENCE_BUCKETGRAPHUTILS_H
#define MUONINFERENCE_BUCKETGRAPHUTILS_H

#include <vector>
#include <cstdint>
#include <algorithm>
#include <cmath>
#include <unordered_set>

#include <ActsGeometryInterfaces/GeometryContext.h>
#include "MuonSpacePoint/SpacePointContainer.h"
#include "MuonSpacePoint/SpacePointPerLayerSorter.h"
#include "GeoPrimitives/GeoPrimitives.h"

namespace MuonML::BucketGraphUtils {

struct NodeAux {
  double x{0.f};
  double y{0.f};
  double z{0.f};
  int   layers{0};
  int   nSp{0};
  double bucketSize{0.f};
  int   sector{0};
  int   chamber{0};
};

inline double bucket_size_mm(const MuonR4::SpacePointBucket& b) {
  return b.coveredMax() - b.coveredMin();
}

/**
 * Build nodes + flat features (N,6) and number of SPs per kept bucket.
 * - Features: [x, y, z, layers, nSp, bucketSize]
 */
inline void buildNodesAndFeatures(const MuonR4::SpacePointContainer& buckets,
                                  const ActsTrk::GeometryContext& gctx,
                                  std::vector<NodeAux>& nodes,
                                  std::vector<float>& featuresLeaves,
                                  std::vector<int64_t>& spInBucket)  // << int64_t!
{
  /// In cas that the data vectors have been used by an earlier ML algorithm ensure that the vectors remain clean
  nodes.clear();
  featuresLeaves.clear();
  spInBucket.clear();
  nodes.reserve(buckets.size());
  featuresLeaves.reserve(6u * buckets.size());   // preallocate
  spInBucket.reserve(buckets.size());

  MuonR4::SpacePointPerLayerSorter layerSorter{};

  for (const MuonR4::SpacePointBucket* bucket : buckets) {
    const double bsize = bucket_size_mm(*bucket);
    // Don't skip buckets with 0 size - this was causing 0 nodes with new gctx implementation
    // The new gctx may result in buckets with exactly 0.0 size that should still be processed
    
    NodeAux n;

    if (bucket->msSector()) {
      const double midY = 0.5 * (bucket->coveredMin() + bucket->coveredMax());
      const Amg::Vector3D glob = bucket->msSector()->localToGlobalTrans(gctx) * (midY * Amg::Vector3D::UnitY());
      n.x = glob.x();
      n.y = glob.y();
      n.z = glob.z();
    }

    std::unordered_set<unsigned int> laySet;
    laySet.reserve(bucket->size());
    for (const auto& spPtr : *bucket) {
      laySet.insert(layerSorter.sectorLayerNum(*spPtr));
    }
    n.layers     = static_cast<int>(laySet.size());
    n.nSp        = static_cast<int>(bucket->size());
    n.bucketSize = bsize;
    n.sector     = bucket->msSector()->sector();
    n.chamber    = Acts::toUnderlying(bucket->msSector()->chamberIndex());

    nodes.push_back(n);

    featuresLeaves.push_back(static_cast<float>(n.x));
    featuresLeaves.push_back(static_cast<float>(n.y));
    featuresLeaves.push_back(static_cast<float>(n.z));
    featuresLeaves.push_back(static_cast<float>(n.layers));
    featuresLeaves.push_back(static_cast<float>(n.nSp));
    featuresLeaves.push_back(static_cast<float>(n.bucketSize));

    spInBucket.emplace_back(static_cast<int64_t>(n.nSp));  // store as int64_t
  }
}

inline void buildSparseEdges(const std::vector<NodeAux>& nodes,
                             int minLayers,
                             int maxChamberDelta,
                             int maxSectorDelta,
                             double maxDistXY,
                             double maxAbsDz,
                             std::vector<int64_t>& srcEdges,
                             std::vector<int64_t>& dstEdges)
{
  /// Ensure that previously built graphs don't leak memory
  srcEdges.clear();
  dstEdges.clear();

  std::vector<size_t> validIdx;
  validIdx.reserve(nodes.size());
  for (size_t i = 0; i < nodes.size(); ++i) {
    if (nodes[i].layers >= minLayers) validIdx.push_back(i);
  }

  if (validIdx.size() < 2) {
    if (!nodes.empty()) {
      srcEdges.push_back(0);
      dstEdges.push_back(0);
    }
    return;
  }

  const unsigned int secMax = Muon::MuonStationIndex::numberOfSectors();

  for (size_t a = 0; a < validIdx.size(); ++a) {
    const size_t i = validIdx[a];
    const auto& ni = nodes[i];
    for (size_t b = a + 1; b < validIdx.size(); ++b) {
      const size_t j = validIdx[b];
      const auto& nj = nodes[j];

      const double dx = static_cast<double>(ni.x) - static_cast<double>(nj.x);
      const double dy = static_cast<double>(ni.y) - static_cast<double>(nj.y);
      const double dz = static_cast<double>(ni.z) - static_cast<double>(nj.z);

      const double distXY  = Acts::fastHypot(dx, dy);
      const int secDiffLin = std::abs(ni.sector - nj.sector) % static_cast<int>(secMax);
      const int d_sec      = std::min(secDiffLin, static_cast<int>(secMax) - secDiffLin);
      const int d_ch       = std::abs(ni.chamber - nj.chamber);

      const bool mask =
        (d_ch > 0) &&
        (d_sec <= maxSectorDelta) &&
        (distXY < maxDistXY) &&
        (std::abs(dz) < maxAbsDz) &&
        (d_ch <= maxChamberDelta);

      if (mask) {
        srcEdges.push_back(static_cast<int64_t>(i));
        dstEdges.push_back(static_cast<int64_t>(j));
        srcEdges.push_back(static_cast<int64_t>(j));
        dstEdges.push_back(static_cast<int64_t>(i));
      }
    }
  }

  if (srcEdges.empty() && !nodes.empty()) {
    srcEdges.push_back(0);
    dstEdges.push_back(0);
  }
}

inline size_t packEdgeIndex(const std::vector<int64_t>& srcEdges,
                            const std::vector<int64_t>& dstEdges,
                            std::vector<int64_t>& edgeIndexPacked)
{
  const size_t E = srcEdges.size();
  edgeIndexPacked = srcEdges;
  edgeIndexPacked.insert(edgeIndexPacked.end(), dstEdges.begin(), dstEdges.end());
  return E;
}

} // namespace MuonML::BucketGraphUtils

#endif

