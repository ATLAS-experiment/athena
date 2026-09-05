/**
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_Analysis/src/HGTD_AnalysisAlgBase.cxx
 *
 * @author Alexander Leopold <alexander.leopold@cern.ch>
 */

#include "HGTD_AnalysisAlgBase.h"

HGTD_AnalysisAlgBase::HGTD_AnalysisAlgBase(const std::string& name,
                                           ISvcLocator* svc_locator)
    : AthAlgorithm(name, svc_locator) {}

HGTD_AnalysisAlgBase::~HGTD_AnalysisAlgBase() {}

StatusCode HGTD_AnalysisAlgBase::initialize() {
  ATH_MSG_INFO("Initializing HGTD_AnalysisAlgBase ...");

  ATH_CHECK(m_hist_svc.retrieve());

  return StatusCode::SUCCESS;
}

StatusCode HGTD_AnalysisAlgBase::finalize() {
  ATH_MSG_INFO("Finalizing HGTD_AnalysisAlgBase ...");
  return StatusCode::SUCCESS;
}

StatusCode HGTD_AnalysisAlgBase::execute(const EventContext& /*ctx*/) {
  ATH_MSG_DEBUG("Executing HGTD_AnalysisAlgBase ...");

  return StatusCode::SUCCESS;
}
