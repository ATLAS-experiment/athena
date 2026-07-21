/**
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_Calibration/src/HGTD_TdcCalibCondAlg.h
 *
 * @author Yuriy Volkotrub <yuriy.volkotrub@cern.ch>
 *
 * @date 2025
 *
 * @brief Conditions algorithm that reads TDC calibration data from the
 *        conditions database (CREST or COOL) and produces an
 *        HGTD_TdcCalibData conditions object.
 *
 * Phase 1: Reads a single toa_bin_size float from CondAttrListCollection (channel 0).
 * Phase 2: Will read per-pixel calibration maps.
 */

#ifndef HGTD_TDCCALIBCONDALG_H
#define HGTD_TDCCALIBCONDALG_H

#include "AthenaBaseComps/AthCondAlgorithm.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "HGTD_Calibration/HGTD_TdcCalibData.h"

class HGTD_TdcCalibCondAlg : public AthCondAlgorithm {
public:
  HGTD_TdcCalibCondAlg(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~HGTD_TdcCalibCondAlg() override = default;

  virtual StatusCode initialize() override final;
  virtual StatusCode execute(const EventContext& ctx) const override final;

private:
  /// Input: raw conditions data from IOVDbSvc (CREST or COOL backend)
  SG::ReadCondHandleKey<CondAttrListCollection> m_readKey{
      this, "ReadKey", "/HGTD/Calibration/TdcBinSize",
      "Key of input CondAttrListCollection from conditions DB"};

  /// Output: typed calibration data object
  SG::WriteCondHandleKey<HGTD_TdcCalibData> m_writeKey{
      this, "WriteKey", "HGTD_TdcCalibData",
      "Key of output HGTD_TdcCalibData conditions object"};
};

#endif // HGTD_TDCCALIBCONDALG_H
