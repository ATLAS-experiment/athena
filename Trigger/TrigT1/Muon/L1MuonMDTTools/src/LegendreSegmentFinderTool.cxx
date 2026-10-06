/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file LegendreSegmentFinderTool.cxx
 *
 * Temporary and simplified Athena-native Legendre segment finder for MDT studies.
 *
 * This implementation is intended only as an early integration and validation tool.
 * It provides a first Legendre-transform-based segment-finding chain in the (z, R) plane,
 * using the same external interface as the CSF-based segment finder.
 *
 * Important limitations of the current implementation:
 *   - this is not a 1:1 port of the old standalone implementation
 *   - only a simplified (z, R) treatment is used
 *   - no phi-sector-dependent corrections are applied
 *   - no dedicated window offsets or geometry-dependent tuning are included
 *   - no smearing or sliding-window logic is included
 *   - no ambiguity-resolution refinements beyond the current simple bin logic are included
 *   - the output segment container is not yet fully populated; at present this tool is mainly
 *     used to validate the Legendre maximum and fitted line parameters
 *
 * Main steps:
 *   1. Build compact hit information from MDT drift circles
 *   2. Define a Legendre window around the RPC seed
 *   3. Fill the sinogram using the two left/right ambiguity branches
 *   4. Find the most populated bin
 *   5. Convert the maximum bin back to a line R = m*z + b
 *   6. Optionally refine the line with a straight-line fit to tangent points
 *
 * This tool is expected to be refined in follow-up work.
 */

#include "LegendreSegmentFinderTool.h"
#include "L1MuonMDTTools/L0MDTSegment.h"
#include "MuonReadoutGeometryR4/MdtReadoutElement.h"
#include "CxxUtils/trapping_fp.h"
#include <algorithm>
#include <cmath>
#include <numeric>

namespace L1Muon::L1MDT {

  StatusCode LegendreSegmentFinderTool::initialize() {
    ATH_MSG_DEBUG("Initializing " << name() << "...");

    ATH_MSG_WARNING(
        "LegendreSegmentFinderTool is currently a temporary and simplified validation "
        "implementation. It is not a full standalone-equivalent Legendre segment finder, "
        "and the output segment container is not yet fully populated.");

    return StatusCode::SUCCESS;
  }


  StatusCode LegendreSegmentFinderTool::buildHitInfo(
      const std::vector<const xAOD::MdtDriftCircle*>& driftCircles,
      const ActsTrk::GeometryContext& gctx,
      std::vector<HitInfo>& hitInfos) const {

    hitInfos.clear();
    hitInfos.reserve(driftCircles.size());

    for (const xAOD::MdtDriftCircle* dc : driftCircles) {
      if (!dc) continue;

      const MuonGMR4::MdtReadoutElement* detEl = dc->readoutElement();
      if (!detEl) continue;

      // Build global hit coordinates from the MDT measurement
      const Amg::Transform3D& locToGlob =
          detEl->localToGlobalTransform(gctx, dc->measurementHash());
      const Amg::Vector3D gpos = locToGlob * dc->localMeasurementPos();

      HitInfo info;
      info.dc = dc;
      info.z = static_cast<float>(gpos.z());
      info.R = static_cast<float>(gpos.perp());
      info.driftRadius = std::abs(dc->driftRadius());

      hitInfos.push_back(info);
    }
    return StatusCode::SUCCESS;
  }


  LegendreSegmentFinderTool::LegendreSpacePars
  LegendreSegmentFinderTool::setLegendreSpacePars(float seedM, float seedB) const {

    LegendreSpacePars pars;

    // Temporary simplified conversion from seed line to Legendre-space coordinates.
    // This parameterization is currently used only to define a validation window
    // around the RPC seed and may need to be refined in future updates.
    pars.seedTheta = std::atan2(-1.f, seedM);

    // Temporary local intercept model used to center the r window.
    // This is part of the current simplified implementation and is not yet
    // a fully validated detector- or sector-aware treatment.
    const float bLocal = seedM + seedB;

    pars.seedR = bLocal * std::sin(pars.seedTheta);

    const float thetaAxisSize = static_cast<float>(m_thetaBins) * m_thetaRes;
    const float rAxisSize     = static_cast<float>(m_rBins) * m_rRes;

    pars.thetaMin = pars.seedTheta - 0.5f * thetaAxisSize;
    pars.thetaMax = pars.seedTheta + 0.5f * thetaAxisSize;

    pars.rMin = pars.seedR - 0.5f * rAxisSize;
    pars.rMax = pars.seedR + 0.5f * rAxisSize;

    return pars;
  }


  int LegendreSegmentFinderTool::findBin(float value, float min, float binSize, int nBins) const {
    const int bin = static_cast<int>(std::floor((value - min) / binSize));
    if (bin < 0 || bin >= nBins) return -1;
    return bin;
  }


  void LegendreSegmentFinderTool::fillBin(
      std::vector<std::vector<BinCell>>& bins,
      int thetaBin,
      int rBin,
      const xAOD::MdtDriftCircle* dc,
      float zPoint,
      float rPoint) const {

    if (thetaBin < 0 || rBin < 0) return;
    if (thetaBin >= static_cast<int>(bins.size())) return;
    if (rBin >= static_cast<int>(bins[thetaBin].size())) return;

    BinCell& cell = bins[thetaBin][rBin];

    // Avoid double-counting the same MDT hit in the same Legendre bin
    auto it = std::find(cell.hits.begin(), cell.hits.end(), dc);
    if (it != cell.hits.end()) return;

    cell.hits.push_back(dc);
    cell.zVals.push_back(zPoint);
    cell.RVals.push_back(rPoint);
    ++cell.entries;
  }


  void LegendreSegmentFinderTool::fillSinogram(
      const std::vector<HitInfo>& hitInfos,
      float thetaMin,
      float rMin,
      std::vector<std::vector<BinCell>>& bins) const {

    bins.assign(m_thetaBins, std::vector<BinCell>(m_rBins));

    for (const HitInfo& hit : hitInfos) {
      for (int iTheta = 0; iTheta < m_thetaBins; ++iTheta) {
        const float theta = thetaMin + (static_cast<float>(iTheta) + 0.5f) * m_thetaRes;

        const float c = std::cos(theta);
        const float s = std::sin(theta);

        // Temporary simplified treatment of the left/right ambiguity:
        // both tangent branches are filled symmetrically in Legendre space.
        const float rPlus  = hit.z * c + hit.R * s + hit.driftRadius;
        const float rMinus = hit.z * c + hit.R * s - hit.driftRadius;

        const float zPlus = hit.z + hit.driftRadius * c;
        const float RPlus = hit.R + hit.driftRadius * s;

        const float zMinus = hit.z - hit.driftRadius * c;
        const float RMinus = hit.R - hit.driftRadius * s;

        const int rBinPlus  = findBin(rPlus,  rMin, m_rRes, m_rBins);
        const int rBinMinus = findBin(rMinus, rMin, m_rRes, m_rBins);

        if (rBinPlus >= 0) {
          fillBin(bins, iTheta, rBinPlus, hit.dc, zPlus, RPlus);
        }
        if (rBinMinus >= 0) {
          fillBin(bins, iTheta, rBinMinus, hit.dc, zMinus, RMinus);
        }
      }
    }
  }


  bool LegendreSegmentFinderTool::findMaxBin(
      const std::vector<std::vector<BinCell>>& bins,
      MaxBin& maxBin) const {

    maxBin = MaxBin{};

    int maxEntries = 0;
    std::vector<int> thetaMaxBins;
    std::vector<int> rMaxBins;

    for (int iTheta = 0; iTheta < static_cast<int>(bins.size()); ++iTheta) {
      for (int iR = 0; iR < static_cast<int>(bins[iTheta].size()); ++iR) {
        const int entries = bins[iTheta][iR].entries;
        if (entries < m_minEntriesInMaxBin) continue;

        if (entries > maxEntries) {
          maxEntries = entries;
          thetaMaxBins.clear();
          rMaxBins.clear();
          thetaMaxBins.push_back(iTheta);
          rMaxBins.push_back(iR);
        } else if (entries == maxEntries) {
          thetaMaxBins.push_back(iTheta);
          rMaxBins.push_back(iR);
        }
      }
    }

    if (maxEntries < m_minEntriesInMaxBin || thetaMaxBins.empty()) return false;

    const float thetaMean =
        std::accumulate(thetaMaxBins.begin(), thetaMaxBins.end(), 0.f) /
        static_cast<float>(thetaMaxBins.size());

    const float rMean =
        std::accumulate(rMaxBins.begin(), rMaxBins.end(), 0.f) /
        static_cast<float>(rMaxBins.size());

    maxBin.thetaBin = static_cast<int>(std::lround(thetaMean));
    maxBin.rBin     = static_cast<int>(std::lround(rMean));
    maxBin.entries  = maxEntries;
    maxBin.valid    = true;

    return true;
  }


  LegendreSegmentFinderTool::FitResult
  LegendreSegmentFinderTool::extractSegmentFromMaxBin(
      const MaxBin& maxBin,
      float thetaMin,
      float rMin) const {

    // Tell clang that FP operations may trap.
    CXXUTILS_TRAPPING_FP;

    FitResult out;
    if (!maxBin.valid) return out;

    const float theta = thetaMin + (static_cast<float>(maxBin.thetaBin) + 0.5f) * m_thetaRes;
    const float r     = rMin     + (static_cast<float>(maxBin.rBin)     + 0.5f) * m_rRes;

    // Convert the temporary Legendre maximum back to a line in the (z, R) plane
    if (std::abs(std::sin(theta)) < 1e-6f) return out;

    out.m = -1.f / std::tan(theta);
    out.b = r / std::sin(theta);
    out.valid = true;

    return out;
  }


  LegendreSegmentFinderTool::FitResult
  LegendreSegmentFinderTool::fitLine(
      const std::vector<float>& zVals,
      const std::vector<float>& RVals,
      float sigma) const {

    FitResult out;

    const std::size_t n = zVals.size();
    if (n < 2 || RVals.size() != n) return out;

    float sumZ  = 0.f;
    float sumR  = 0.f;
    float sumZZ = 0.f;
    float sumZR = 0.f;

    for (std::size_t i = 0; i < n; ++i) {
      sumZ  += zVals[i];
      sumR  += RVals[i];
      sumZZ += zVals[i] * zVals[i];
      sumZR += zVals[i] * RVals[i];
    }

    const float N   = static_cast<float>(n);
    const float den = N * sumZZ - sumZ * sumZ;
    if (std::abs(den) < 1e-6f) return out;

    out.m = (N * sumZR - sumZ * sumR) / den;
    out.b = (sumR - out.m * sumZ) / N;

    float chi2 = 0.f;
    for (std::size_t i = 0; i < n; ++i) {
      const float res = (RVals[i] - (out.m * zVals[i] + out.b)) / sigma;
      chi2 += res * res;
    }

    out.chi2 = chi2;
    out.valid = true;
    return out;
  }


  StatusCode LegendreSegmentFinderTool::findSegments(
      const std::vector<const xAOD::MdtDriftCircle*>& driftCircles,
      const ActsTrk::GeometryContext& gctx,
      float m,
      float b,
      std::vector<L1Muon::L1MDT::Segment>& segments) const {

    segments.clear();

    ATH_MSG_DEBUG("In LegendreSegmentFinderTool::findSegments()");
    ATH_MSG_DEBUG("Input drift circles: " << driftCircles.size()
                  << ", current output segments: " << segments.size());

    // Step 1: build compact hit information
    std::vector<HitInfo> hitInfos;
    ATH_CHECK(buildHitInfo(driftCircles, gctx, hitInfos));

    if (hitInfos.size() < 2) {
      ATH_MSG_DEBUG("Not enough hits for temporary Legendre segment finding");
      return StatusCode::SUCCESS;
    }

    // Step 2: define the Legendre window around the RPC seed
    const auto pars = setLegendreSpacePars(m, b);

    if (m_debugLegendre) {
      ATH_MSG_INFO("Temporary Legendre seed window | "
                   << "seedTheta=" << pars.seedTheta
                   << " seedR=" << pars.seedR
                   << " thetaMin=" << pars.thetaMin
                   << " thetaMax=" << pars.thetaMax
                   << " rMin=" << pars.rMin
                   << " rMax=" << pars.rMax);
    }

    // Step 3: fill the sinogram
    std::vector<std::vector<BinCell>> bins;
    fillSinogram(hitInfos, pars.thetaMin, pars.rMin, bins);

    // Step 4: find the maximum
    MaxBin maxBin;
    if (!findMaxBin(bins, maxBin)) {
      ATH_MSG_DEBUG("No valid temporary Legendre maximum found");
      return StatusCode::SUCCESS;
    }

    // Step 5: convert the maximum bin into a first line estimate
    FitResult fit = extractSegmentFromMaxBin(maxBin, pars.thetaMin, pars.rMin);
    if (!fit.valid) {
      ATH_MSG_DEBUG("Failed to convert temporary Legendre maximum into a line estimate");
      return StatusCode::SUCCESS;
    }

    const float thetaMaxCenter =
        pars.thetaMin + (static_cast<float>(maxBin.thetaBin) + 0.5f) * m_thetaRes;
    const float rMaxCenter =
        pars.rMin + (static_cast<float>(maxBin.rBin) + 0.5f) * m_rRes;

    // Step 6: optional straight-line refit using tangent points stored in the max bin
    const BinCell& bestCell = bins[maxBin.thetaBin][maxBin.rBin];
    if (m_doRefit && bestCell.zVals.size() >= 2) {
      FitResult refit = fitLine(bestCell.zVals, bestCell.RVals);
      if (refit.valid) {
        fit = refit;
      }
    }

    ATH_MSG_DEBUG("Temporary LT line estimate | "
                  << "theta=" << thetaMaxCenter
                  << " r=" << rMaxCenter
                  << " m=" << fit.m
                  << " b=" << fit.b
                  << " chi2=" << fit.chi2
                  << " nHits=" << bestCell.zVals.size());

    // Temporary validation-only status:
    // the fitted line parameters are computed and logged, but the final
    // L1Muon::L1MDT::Segment EDM object is not yet fully filled and stored.
    //
    // Future updates are expected to:
    //   - define the final segment EDM content
    //   - populate the segment object consistently
    //   - validate the parameterization and quality variables
    //
    // Example placeholder:
    //
    // L1Muon::L1MDT::Segment seg;
    // seg.setM(fit.m);
    // seg.setB(fit.b);
    // seg.setChi2(fit.chi2);
    // seg.setNHits(bestCell.zVals.size());
    // segments.push_back(seg);

    ATH_MSG_DEBUG("Temporary LegendreSegmentFinderTool finished without populating the final segment container");

    return StatusCode::SUCCESS;
  }

} // namespace L1Muon::L1MDT

