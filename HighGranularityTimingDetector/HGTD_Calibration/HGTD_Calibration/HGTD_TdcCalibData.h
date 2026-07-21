/**
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_Calibration/HGTD_Calibration/HGTD_TdcCalibData.h
 *
 * @author Yuriy Volkotrub <yuriy.volkotrub@cern.ch>
 *
 * @date 2025
 *
 * @brief Conditions data object holding TDC calibration constants.
 *
 * Phase 1: Stores a single global toa_bin_size value (replacing the
 *          hardcoded 20 ps default in HGTD_TdcCalibrationTool).
 *
 * Phase 2 (future): Will be extended to hold per-pixel, per-bin
 *          calibration data keyed by detector identifier.
 */

#ifndef HGTD_TDCCALIBDATA_H
#define HGTD_TDCCALIBDATA_H

#include "AthenaKernel/CLASS_DEF.h"
#include "AthenaKernel/CondCont.h"

class HGTD_TdcCalibData {
public:
  HGTD_TdcCalibData() = default;
  ~HGTD_TdcCalibData() = default;

  /** @brief Set the global TOA bin size (in ns). */
  void setToaBinSize(float value) { m_toaBinSize = value; }

  /** @brief Get the global TOA bin size (in ns).
   *  Phase 1: returns the single stored value.
   *  Phase 2: this method will be extended/overloaded to accept
   *           a pixel identifier and return per-pixel values.
   */
  float toaBinSize() const { return m_toaBinSize; }

  // ── Phase 2 placeholder ──
  // float toaBinSize(const Identifier& pixelId) const;
  // float toaBinSize(const Identifier& pixelId, uint8_t toaBin) const;

private:
  float m_toaBinSize{0.02f};  // default: 20 ps = 0.02 ns
};

// Register for conditions store
CLASS_DEF(HGTD_TdcCalibData, 82947356, 1)
CONDCONT_DEF(HGTD_TdcCalibData, 82947357);

#endif // HGTD_TDCCALIBDATA_H
