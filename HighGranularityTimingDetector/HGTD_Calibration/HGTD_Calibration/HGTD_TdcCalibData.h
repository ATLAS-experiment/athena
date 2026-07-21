/**
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_Calibration/HGTD_Calibration/HGTD_TdcCalibData.h
 *
 * @author Yuriy Volkotrub <yuriy.volkotrub@cern.ch>
 *
 * @date 2025
 *
 * @brief Conditions data object holding TDC calibration constants.
 *
 * Phase 1 stores a single global TOA bin size in ns.
 */

#ifndef HGTD_TDCCALIBDATA_H
#define HGTD_TDCCALIBDATA_H

#include "AthenaKernel/CLASS_DEF.h"
#include "AthenaKernel/CondCont.h"

class HGTD_TdcCalibData {
public:
  explicit HGTD_TdcCalibData(float toaBinSize)
      : m_toaBinSize(toaBinSize) {}
  ~HGTD_TdcCalibData() = default;

  /** @brief Get the global TOA bin size in ns. */
  float toaBinSize() const { return m_toaBinSize; }

private:
  float m_toaBinSize;
};

CLASS_DEF(HGTD_TdcCalibData, 82947356, 1)
CONDCONT_DEF(HGTD_TdcCalibData, 82947357);

#endif // HGTD_TDCCALIBDATA_H
