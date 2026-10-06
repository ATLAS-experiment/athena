/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file PtEstimationTool.cxx
 *
 * Temporary and explicitly simplified pT-proxy tool for L1Muon MDT studies.
 *
 * Important limitations of the current implementation:
 *   - no magnetic-field model is used
 *   - no charge-sign handling is included
 *   - no geometry-dependent calibration is applied
 *   - no material effects / energy loss are considered
 *   - no alignment / detector distortions are considered
 *   - the returned quantity is only a geometric proxy, not a calibrated pT
 *
 * The purpose of this tool is only to provide a simple and transparent
 * placeholder during early integration and validation of the segment-finding chain.
 * It is expected to be replaced or significantly refined in follow-up work.
 */

#include "PtEstimationTool.h"

#include <algorithm>
#include <cmath>

namespace {

// Convert a segment slope m into an angle in the (z, R) plane
float thetaFromSlope(float m) {
  return std::atan(m);
}

// Compute the signed distance of point (z, r) from the line through (z1, r1) and (z2, r2)
float pointToLineDistance(float z, float r,
                          float z1, float r1,
                          float z2, float r2) {
  const float dz = z2 - z1;
  const float dr = r2 - r1;
  const float den = std::hypot(dz, dr);

  // Protect against degenerate input geometry
  if (den < 1.e-6f) {
    return 0.f;
  }

  return ((z - z1) * dr - (r - r1) * dz) / den;
}

}  // namespace

namespace L1Muon::L1MDT {

StatusCode PtEstimationTool::initialize() {
  ATH_MSG_DEBUG("Initializing " << name());

  return StatusCode::SUCCESS;
}

std::optional<PtEstimate> PtEstimationTool::estimatePt(const Segment* biSeg,
                                                       const Segment* bmSeg,
                                                       const Segment* boSeg) const {
  const bool hasBI = (biSeg != nullptr);
  const bool hasBM = (bmSeg != nullptr);
  const bool hasBO = (boSeg != nullptr);

  const unsigned int nStations = static_cast<unsigned int>(hasBI) +
                                 static_cast<unsigned int>(hasBM) +
                                 static_cast<unsigned int>(hasBO);

  // A meaningful geometric proxy needs at least two stations
  if (nStations < 2) {
    ATH_MSG_DEBUG("PtEstimationTool: fewer than 2 stations available, no pT proxy computed");
    return std::nullopt;
  }

  PtEstimate result{};
  result.nStations = nStations;

  // --------------------------------------------------------------------------
  // Two-station mode
  //
  // This branch estimates a toy pT proxy from the change in segment angle.
  // This is only a bending proxy and is not a calibrated momentum estimate.
  // --------------------------------------------------------------------------
  if (nStations == 2) {
    const Segment* inner = nullptr;
    const Segment* outer = nullptr;

    if (hasBI && hasBM) {
      inner = biSeg;
      outer = bmSeg;
    } else if (hasBI && hasBO) {
      inner = biSeg;
      outer = boSeg;
    } else if (hasBM && hasBO) {
      inner = bmSeg;
      outer = boSeg;
    }

    if (!inner || !outer) {
      ATH_MSG_WARNING("PtEstimationTool: invalid 2-station configuration");
      return std::nullopt;
    }

    const float thetaInner = thetaFromSlope(inner->m());
    const float thetaOuter = thetaFromSlope(outer->m());

    result.deltaBeta = thetaOuter - thetaInner;
    result.pt = estimateTwoStationPt(result.deltaBeta);

    ATH_MSG_DEBUG("Temporary 2-station pT proxy | "
                  << "deltaBeta=" << result.deltaBeta
                  << " ptProxy=" << result.pt);

    return result;
  }

  // --------------------------------------------------------------------------
  // Three-station mode
  //
  // This branch estimates a toy pT proxy from sagitta and lever arm.
  // The result is purely geometric and ignores magnetic field and calibration.
  // --------------------------------------------------------------------------
  const float sagitta = pointToLineDistance(
      bmSeg->zRef(), bmSeg->rRef(),
      biSeg->zRef(), biSeg->rRef(),
      boSeg->zRef(), boSeg->rRef());

  const float leverArm = std::hypot(
      boSeg->zRef() - biSeg->zRef(),
      boSeg->rRef() - biSeg->rRef());

  result.sagitta = sagitta;
  result.leverArm = leverArm;
  result.pt = estimateThreeStationPt(result.sagitta, result.leverArm);

  ATH_MSG_DEBUG("Temporary 3-station pT proxy | "
                << "sagitta=" << result.sagitta
                << " leverArm=" << result.leverArm
                << " ptProxy=" << result.pt);

  return result;
}

// Convert a 2-station angular deflection into a toy pT proxy
float PtEstimationTool::estimateTwoStationPt(float deltaBeta) const {
  // Protection against vanishing angular difference
  constexpr float minAbsDeltaBeta = 1.e-4f;

  // Purely ad hoc scale factor for a temporary proxy
  constexpr float kToyPtScale = 1000.f;

  const float absDeltaBeta = std::max(std::abs(deltaBeta), minAbsDeltaBeta);

  // Temporary proxy: larger bending -> lower pT proxy
  return kToyPtScale / absDeltaBeta;
}

// Convert a 3-station sagitta into a toy pT proxy
float PtEstimationTool::estimateThreeStationPt(float sagitta, float leverArm) const {
  // Protection against vanishing sagitta
  constexpr float minAbsSagitta = 1.e-3f;

  // Purely ad hoc scale factor for a temporary proxy
  constexpr float kToyPtScale = 1.e-3f;

  const float absSagitta = std::max(std::abs(sagitta), minAbsSagitta);

  // Temporary proxy: pT ~ L^2 / sagitta
  return kToyPtScale * leverArm * leverArm / absSagitta;
}

}  // namespace L1Muon::L1MDT

