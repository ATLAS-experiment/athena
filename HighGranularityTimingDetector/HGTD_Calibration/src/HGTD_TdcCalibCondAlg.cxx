/**
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_Calibration/src/HGTD_TdcCalibCondAlg.cxx
 *
 * @author Yuriy Volkotrub <yuriy.volkotrub@cern.ch>
 *
 * @date 2025
 *
 * @brief Reads TDC calibration data from conditions DB and produces
 *        HGTD_TdcCalibData conditions object.
 */

#include "HGTD_TdcCalibCondAlg.h"
#include "AthenaKernel/IOVInfiniteRange.h"

HGTD_TdcCalibCondAlg::HGTD_TdcCalibCondAlg(const std::string& name,
                                             ISvcLocator* pSvcLocator)
    : AthCondAlgorithm(name, pSvcLocator) {}

StatusCode HGTD_TdcCalibCondAlg::initialize() {
  ATH_MSG_DEBUG("initialize " << name());

  ATH_CHECK(m_readKey.initialize());
  ATH_CHECK(m_writeKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode HGTD_TdcCalibCondAlg::execute(const EventContext& ctx) const {
  ATH_MSG_DEBUG("execute " << name());

  // ── Check if output is already valid ──
  SG::WriteCondHandle<HGTD_TdcCalibData> writeHandle{m_writeKey, ctx};
  if (writeHandle.isValid()) {
    ATH_MSG_DEBUG("CondHandle " << writeHandle.fullKey()
                  << " is already valid. Skipping.");
    return StatusCode::SUCCESS;
  }

  // ── Read input conditions data ──
  SG::ReadCondHandle<CondAttrListCollection> readHandle{m_readKey, ctx};
  if (!readHandle.isValid()) {
    ATH_MSG_FATAL("Could not read conditions data from " << m_readKey.key());
    return StatusCode::FAILURE;
  }

  // Propagate the IOV range from the input
  writeHandle.addDependency(readHandle);

  const CondAttrListCollection* attrListColl = readHandle.cptr();

  // ── Extract calibration values ──
  auto calibData = std::make_unique<HGTD_TdcCalibData>();

  // Phase 1: single global toa_bin_size value from channel 0
  auto chanIt = attrListColl->chanAttrListPair(0);  // channel 0
  if (chanIt != attrListColl->end()) {
    const coral::AttributeList& attrList = chanIt->second;
    if (attrList.exists("toa_bin_size")) {
      float toaBinSize = attrList["toa_bin_size"].data<float>();
      calibData->setToaBinSize(toaBinSize);
      ATH_MSG_INFO("Read toa_bin_size = " << toaBinSize
                   << " ns (" << toaBinSize * 1000 << " ps) from conditions DB");
    } else {
      ATH_MSG_WARNING("Attribute 'toa_bin_size' not found in channel 0. "
                      "Using default value: " << calibData->toaBinSize() << " ns");
    }
  } else {
    ATH_MSG_WARNING("Channel 0 not found in CondAttrListCollection. "
                    "Using default value: " << calibData->toaBinSize() << " ns");
  }

  // ── Record the output ──
  if (writeHandle.record(std::move(calibData)).isFailure()) {
    ATH_MSG_FATAL("Could not record " << writeHandle.key()
                  << " with range " << writeHandle.getRange()
                  << " into Conditions Store");
    return StatusCode::FAILURE;
  }

  ATH_MSG_INFO("Recorded HGTD_TdcCalibData with range "
               << writeHandle.getRange());

  return StatusCode::SUCCESS;
}
