/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef L1MUONNSW_NSWENCODING_H
#define L1MUONNSW_NSWENCODING_H

#include "xAODTrigL1Muon/NSWTPBits.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <numbers>

/**
 * @brief Quantisation of NSW segment quantities into the Run 4 NSW-TP -> TGC-SL segment word
 *        (L1Muon interface document, EDMS 2377909, Section 4.2, Figure 13).
 *
 * Kept outside the xAOD class (which only packs/unpacks bits) and outside the trigger tools
 * (which produce physics values), so that MM and later sTGC segments share one encoding.
 * Each xxxIndex() has a matching xxxValue() that returns the value the index stands for,
 * used for validation.
 */
namespace L1Muon::NSWEncoding {

  constexpr double kTwoPi = 2. * std::numbers::pi;

  /// Map any angle to [0, 2pi)
  inline double wrapTwoPi(double phi) {
    phi = std::fmod(phi, kTwoPi);
    return (phi < 0.) ? phi + kTwoPi : phi;
  }

  // ---------------------------------------------------------------------------------------
  // eta: 14 bits over [ETA_MIN, ETA_MAX]; nearest bin. Same scale as L1NSWCandData_v1::segEta().
  // The NSW covers 1.3 < |eta| < 2.7, so inRange is false only for a broken input.
  // ---------------------------------------------------------------------------------------
  inline uint16_t etaIndex(double eta, bool& inRange) {
    using namespace L1Muon::NSWTPBits;
    const double x = (eta - ETA_MIN) / (ETA_MAX - ETA_MIN) * ETA_MAX_RAW;
    inRange = (x >= -0.5 && x <= ETA_MAX_RAW + 0.5);
    return static_cast<uint16_t>(std::clamp(std::round(x), 0., static_cast<double>(ETA_MAX_RAW)));
  }
  inline double etaValue(uint16_t idx) {
    using namespace L1Muon::NSWTPBits;
    return ETA_MIN + (idx / static_cast<double>(ETA_MAX_RAW)) * (ETA_MAX - ETA_MIN);
  }

  // ---------------------------------------------------------------------------------------
  // phi: 10 bits over [0, 2pi]; nearest bin. Same scale as L1NSWCandData_v1::segPhi().
  // PHI_MAX_RAW = 1023, so index 1023 decodes to 2pi, the same direction as index 0. Whether the
  // hardware divides 2pi in 1023 or 1024 steps is to be confirmed with the NSW-TP firmware.
  // Never out of range: the angle is wrapped first.
  // ---------------------------------------------------------------------------------------
  inline uint16_t phiIndex(double phi) {
    using namespace L1Muon::NSWTPBits;
    const double x = wrapTwoPi(phi) / PHI_MAX * PHI_MAX_RAW;
    return static_cast<uint16_t>(std::clamp(std::round(x), 0., static_cast<double>(PHI_MAX_RAW)));
  }
  inline double phiValue(uint16_t idx) {
    using namespace L1Muon::NSWTPBits;
    return (idx / static_cast<double>(PHI_MAX_RAW)) * PHI_MAX;
  }

  // ---------------------------------------------------------------------------------------
  // Delta theta: 5 bits, sign-magnitude. [4] sign (0:+, 1:-), [3:0] |value| in steps of `step`.
  // '+0' is used for 0, '-0' is never produced (interface document, Figure 13).
  // With step = 1 mrad the 4 magnitude bits cover |dtheta| <= 15 mrad, the Run 3 range.
  // Out of range (|dtheta| > range): the segment is KEPT and the magnitude saturates at 15,
  // keeping the sign. (Run 3 encoded out-of-range values as index 0, i.e. -15 mrad, and also
  // kept the segment.) inRange reports it, so it can be counted.
  // ---------------------------------------------------------------------------------------
  constexpr unsigned kDthMagMax = 15;
  constexpr uint8_t kDthSignBit = 0x10;

  inline uint8_t dthetaIndex(double dtheta, double step, double range, bool& inRange) {
    inRange = std::abs(dtheta) <= range;
    const double mag = std::min(std::round(std::abs(dtheta) / step), static_cast<double>(kDthMagMax));
    const uint8_t sign = (dtheta < 0. && mag > 0.) ? kDthSignBit : 0;
    return sign | static_cast<uint8_t>(mag);
  }
  inline double dthetaValue(uint8_t idx, double step) {
    const double mag = (idx & 0xF) * step;
    return (idx & kDthSignBit) ? -mag : mag;
  }

  // ---------------------------------------------------------------------------------------
  // Quality flag (3 bits): 0 invalid/no segment, 1 sTGC pad, 2 sTGC strip, 3 MM, 4 combination
  // ---------------------------------------------------------------------------------------
  constexpr uint8_t kQualityMM = 3;

  // ---------------------------------------------------------------------------------------
  // Board ID (7 bits). Provisional mapping, until the assignment of the NSW-TP boards to the
  // sectors is defined (interface document, Section 4.2): one board per NSW sector and side,
  // boardID = 16*side + (sector-1), side 0 = A, 1 = C  ->  0..31.
  // The Run 4 frame has no side field, so the side has to be carried here.
  // (Run 3 NSW_TrigRawData: sectorId = sector-1, side as a separate char.)
  // ---------------------------------------------------------------------------------------
  inline uint16_t boardID(int sectorPhi, bool isSideC) {
    return static_cast<uint16_t>((isSideC ? 16 : 0) + (sectorPhi - 1));
  }

}  // namespace L1Muon::NSWEncoding

#endif  // L1MUONNSW_NSWENCODING_H
