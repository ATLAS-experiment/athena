/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef L1MuonMDTTools_LEGENDRESEGMENTFINDERTOOL_H
#define L1MuonMDTTools_LEGENDRESEGMENTFINDERTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"

// local includes
#include "xAODMuonPrepData/MdtDriftCircleContainer.h"
#include "L1MuonMDTTools/IL0MDTSegmentFinderTool.h"
#include "L1MuonMDTTools/L0MDTSegment.h"
#include "ActsGeometryInterfaces/GeometryContext.h"

// C++ std
#include <vector>
#include <cstddef>

namespace L1Muon::L1MDT {

class LegendreSegmentFinderTool : public extends<AthAlgTool, IL0MDTSegmentFinderTool> {
public:
  using base_class::base_class;
  virtual ~LegendreSegmentFinderTool() override = default;

  virtual StatusCode initialize() override;

  virtual StatusCode findSegments(const std::vector<const xAOD::MdtDriftCircle*>& driftCircles,
                                  const ActsTrk::GeometryContext& gctx,
                                  float m,
                                  float b,
                                  std::vector<L1Muon::L1MDT::Segment>& segments) const override;

private:
  // Per-hit information in the current reconstruction plane
  struct HitInfo {
    const xAOD::MdtDriftCircle* dc{nullptr};
    float z{0.f};
    float R{0.f};
    float driftRadius{0.f};
  };

  // One Legendre-space bin
  struct BinCell {
    int entries{0};
    std::vector<const xAOD::MdtDriftCircle*> hits;
    std::vector<float> zVals;
    std::vector<float> RVals;
  };

  // Descriptor of the maximum bin
  struct MaxBin {
    int thetaBin{-1};
    int rBin{-1};
    int entries{0};
    bool valid{false};
  };

  // Final fit result
  struct FitResult {
    float m{0.f};
    float b{0.f};
    float chi2{0.f};
    bool valid{false};
  };

  // Parameters defining the Legendre-space window around the RPC seed
  struct LegendreSpacePars {
    float thetaMin{0.f};
    float thetaMax{0.f};
    float rMin{0.f};
    float rMax{0.f};
    float seedTheta{0.f};
    float seedR{0.f};
  };

  StatusCode buildHitInfo(const std::vector<const xAOD::MdtDriftCircle*>& driftCircles,
                          const ActsTrk::GeometryContext& gctx,
                          std::vector<HitInfo>& hitInfos) const;

  LegendreSpacePars setLegendreSpacePars(float seedM, float seedB) const;

  void fillBin(std::vector<std::vector<BinCell>>& bins,
               int thetaBin,
               int rBin,
               const xAOD::MdtDriftCircle* dc,
               float zPoint,
               float rPoint) const;

  void fillSinogram(const std::vector<HitInfo>& hitInfos,
                    float thetaMin,
                    float rMin,
                    std::vector<std::vector<BinCell>>& bins) const;

  bool findMaxBin(const std::vector<std::vector<BinCell>>& bins,
                  MaxBin& maxBin) const;

  FitResult extractSegmentFromMaxBin(const MaxBin& maxBin,
                                     float thetaMin,
                                     float rMin) const;

  FitResult fitLine(const std::vector<float>& zVals,
                    const std::vector<float>& RVals,
                    float sigma = 1.f / 8.f) const;

  int findBin(float value, float min, float binSize, int nBins) const;

private:
  // Minimal Legendre settings for the first Athena implementation
  int   m_thetaBins{64};
  float m_thetaRes{0.002f};
  int   m_rBins{64};
  float m_rRes{2.0f};
  int   m_minEntriesInMaxBin{3};
  bool  m_doRefit{true};
  bool  m_debugLegendre{false};
};

} // namespace L1Muon::L1MDT

#endif