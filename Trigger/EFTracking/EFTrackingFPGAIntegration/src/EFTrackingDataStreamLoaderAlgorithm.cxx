/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#include "EFTrackingDataStreamLoaderAlgorithm.h"

EFTrackingDataStreamLoaderAlgorithm::EFTrackingDataStreamLoaderAlgorithm(
  const std::string& name,
  ISvcLocator* pSvcLocator
) : AthReentrantAlgorithm(name, pSvcLocator)
{}

StatusCode EFTrackingDataStreamLoaderAlgorithm::initialize() {
  ATH_MSG_INFO("Initializing " << name());
  ATH_CHECK(m_inputDataStreamKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode EFTrackingDataStreamLoaderAlgorithm::execute(const EventContext& ctx) const {
  SG::WriteHandle<std::vector<unsigned long>>inputDataStream(
    m_inputDataStreamKey,
    ctx
  );

  ATH_CHECK(inputDataStream.record(std::make_unique<std::vector<unsigned long>>()));
  inputDataStream->reserve(m_bufferSize);

  ATH_CHECK(m_testVectorTool->prepareTV(m_inputCsvPath, *inputDataStream));

  return StatusCode::SUCCESS;
}

