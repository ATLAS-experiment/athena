/*
    Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file src/DataPreparationPipeline.h
 * @author zhaoyuan.cui@cern.ch
 * @author yuan-tang.chou@cern.ch
 * @date Feb. 18, 2024
 */

#include "DataPreparationPipeline.h"

#include <algorithm>

#include "AthContainers/debug.h"
#include "xAODInDetMeasurement/StripClusterAuxContainer.h"

StatusCode DataPreparationPipeline::initialize()
{
  // Check if we are using the pass through tool
  // If yes, check if we are running in software mode. sw mode doesn't require an accelerator
  if (m_usePassThrough)
  {
    ATH_CHECK(m_passThroughTool.retrieve());
    if (m_passThroughTool->runSW())
    {
      ATH_MSG_INFO("Running software pass through");
    }
    else
    {
      ATH_MSG_INFO("Running hardware pass through");
      ATH_CHECK(IntegrationBase::precheck({m_xclbin, m_kernelName}));
      ATH_CHECK(IntegrationBase::initialize());
      ATH_CHECK(IntegrationBase::loadProgram(m_xclbin));
    }
  }
  // We are doing the Data Preparation Pipeline on FPGA, so we need to setup hardware
  else
  {
    ATH_MSG_INFO("Running the DataPrep pipeline on the FPGA accelerator");
    ATH_CHECK(IntegrationBase::precheck({m_xclbin, m_kernelName}));
    ATH_CHECK(IntegrationBase::initialize());
    ATH_CHECK(IntegrationBase::loadProgram(m_xclbin));
  }

  ATH_CHECK(m_xAODContainerMaker.retrieve());

  return StatusCode::SUCCESS;
}

StatusCode DataPreparationPipeline::execute(const EventContext &ctx) const
{
  if (m_usePassThrough)
  {
    EFTrackingDataFormats::StripClusterAuxInput scAux;
    EFTrackingDataFormats::PixelClusterAuxInput pxAux;
    std::unique_ptr<EFTrackingDataFormats::Metadata> metadata =
        std::make_unique<EFTrackingDataFormats::Metadata>();

    ATH_CHECK(m_passThroughTool->runPassThrough(scAux, pxAux, metadata.get(), ctx));
    ATH_CHECK(m_xAODContainerMaker->makeStripClusterContainer(scAux, metadata.get(), ctx));
    ATH_CHECK(m_xAODContainerMaker->makePixelClusterContainer(pxAux, metadata.get(), ctx));

    return StatusCode::SUCCESS;
  }


  // HW kernels to be added here in the future
  ATH_MSG_INFO("Running the FPGA Data Preparation Pipeline");

  return StatusCode::SUCCESS;
}
