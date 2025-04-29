/*
    Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file src/DataPreparationPipeline.h
 * @author zhaoyuan.cui@cern.ch
 * @author yuan-tang.chou@cern.ch
 * @date Feb. 18, 2024
 */

#include "EFTrackingFPGAPipeline/DataPreparationPipeline.h"

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
      ATH_CHECK(IntegrationBase::precheck({m_xclbin, m_passThroughKernelName}));
      ATH_CHECK(IntegrationBase::initialize());
      ATH_CHECK(IntegrationBase::loadProgram(m_xclbin));
    }
  }
  // We are doing the Data Preparation Pipeline on FPGA, so we need to setup hardware
  else
  {
    ATH_MSG_INFO("Running the DataPrep pipeline on the FPGA accelerator");
    ATH_CHECK(IntegrationBase::precheck({m_xclbin, m_pixelClusteringKernelName, m_spacepointKernelName}));
    ATH_CHECK(IntegrationBase::initialize());
    ATH_CHECK(IntegrationBase::loadProgram(m_xclbin));

    // Kernel objects shouldn't change so they can be initialized here
    m_pixelClusteringKernel = cl::Kernel(m_program, m_pixelClusteringKernelName.value().data());
    m_spacepointKernel = cl::Kernel(m_program, m_spacepointKernelName.value().data());

    ATH_CHECK(setupBuffers());
    ATH_CHECK(setupKernelArgs());

    if (m_useTV)
    {
      ATH_CHECK(IntegrationBase::precheck({m_pixelClusterTVPath,
                                           m_pixelClusterRefTVPath,
                                           m_spacepointTVPath,
                                           m_spacepointRefTVPath}));
      ATH_CHECK(m_testVectorTool.retrieve());
    }
  }

  ATH_CHECK(m_xAODClusterMaker.retrieve());
  ATH_CHECK(m_xAODSpacePointMaker.retrieve());

  return StatusCode::SUCCESS;
}

StatusCode DataPreparationPipeline::execute(const EventContext &ctx) const
{
  if (m_usePassThrough)
  {
    EFTrackingTransient::StripClusterAuxInput scAux;
    EFTrackingTransient::PixelClusterAuxInput pxAux;
    
    // Create separate structures for pixel and strip space points
    EFTrackingTransient::SpacePointAuxInput pixelSpAux;
    EFTrackingTransient::SpacePointAuxInput stripSpAux;
    
    std::unique_ptr<EFTrackingTransient::Metadata> metadata =
        std::make_unique<EFTrackingTransient::Metadata>();

    // Pass both aux structures to the PassThroughTool
    ATH_CHECK(m_passThroughTool->runPassThrough(scAux, pxAux, stripSpAux, pixelSpAux, metadata.get(), ctx));
    ATH_CHECK(m_xAODClusterMaker->makeStripClusterContainer(scAux, metadata.get(), ctx));
    ATH_CHECK(m_xAODClusterMaker->makePixelClusterContainer(pxAux, metadata.get(), ctx));

    ATH_CHECK(m_xAODSpacePointMaker->makeStripSpacePointContainer(stripSpAux, metadata.get(), ctx));
    ATH_CHECK(m_xAODSpacePointMaker->makePixelSpacePointContainer(pixelSpAux, metadata.get(), ctx));

    return StatusCode::SUCCESS;
  }

  // HW kernels to be added here in the future
  ATH_MSG_INFO("Running the FPGA Data Preparation Pipeline");

  // Order of execution
  // 1. Pixel/strip clustering
  // 2. Pixel/strip L2G
  // 3. Pixel/strip Spacepoint

  // Are we using TV or RDO?
  if (m_useTV)
  {
    // Make queue of commands
    cl::CommandQueue acc_queue(m_context, m_accelerator);

    ATH_MSG_INFO("Using test vectors");
    EFTrackingFPGAIntegration::TVHolder pixelTV("PixelClustering");
    ATH_CHECK(m_testVectorTool->prepareTV(m_pixelClusterTVPath, pixelTV.inputTV));
    ATH_CHECK(m_testVectorTool->prepareTV(m_pixelClusterRefTVPath, pixelTV.refTV));

    std::vector<uint64_t> fpgaOutput(pixelTV.inputTV.size() * 2, 0);

    ATH_MSG_INFO("Running the pixel clustering kernel");
    acc_queue.enqueueWriteBuffer(m_pxlClusteringInBuff, CL_TRUE, 0, sizeof(uint64_t) * pixelTV.inputTV.size(), pixelTV.inputTV.data(), NULL, NULL);
    acc_queue.enqueueTask(m_pixelClusteringKernel);
    acc_queue.finish();

    // The current implementation of the kernel Read/Write the same buffer
    // So the line below reads the inbuff insatead of outbuff
    acc_queue.enqueueReadBuffer(m_pxlClusteringInBuff, CL_TRUE, 0, sizeof(uint64_t) * fpgaOutput.size(), fpgaOutput.data(), NULL, NULL);

    // compare the output with the reference
    ATH_CHECK(m_testVectorTool->compare(pixelTV, fpgaOutput));

    // do the same for spacepoint
    EFTrackingFPGAIntegration::TVHolder spacepointTV("Spacepoint");
    ATH_CHECK(m_testVectorTool->prepareTV(m_spacepointTVPath, spacepointTV.inputTV));
    ATH_CHECK(m_testVectorTool->prepareTV(m_spacepointRefTVPath, spacepointTV.refTV));

    // reset the output vector
    fpgaOutput = std::vector<uint64_t>(spacepointTV.inputTV.size() * 2, 0);

    ATH_MSG_INFO("Running the spacepoint kernel");
    acc_queue.enqueueWriteBuffer(m_spacepointInBuff, CL_TRUE, 0, sizeof(uint64_t) * spacepointTV.inputTV.size(), spacepointTV.inputTV.data(), NULL, NULL);
    acc_queue.enqueueTask(m_spacepointKernel);
    acc_queue.finish();

    acc_queue.enqueueReadBuffer(m_spacepointInBuff, CL_TRUE, 0, sizeof(uint64_t) * fpgaOutput.size(), fpgaOutput.data(), NULL, NULL);

    // compare the output with the reference
    ATH_CHECK(m_testVectorTool->compare(spacepointTV, fpgaOutput));
  }
  else
  {
    // Use FPGADataFormatTool to prepare input
    ATH_MSG_INFO("Using RDOs for FPGA is not ready yet");
  }

  return StatusCode::SUCCESS;
}

StatusCode DataPreparationPipeline::setupBuffers()
{
  // Allocate buffers on accelerator
  cl_int err = 0;

  // Set the size of the buffer to be 100000, should be further discussed!
  unsigned int size = 100000;
  m_pxlClusteringInBuff = cl::Buffer(m_context, CL_MEM_READ_ONLY, sizeof(uint64_t) * size, NULL, &err);
  if (err != CL_SUCCESS)
  {
    ATH_MSG_ERROR("Failed to allocate intput buffer for pixel clustering");
    return StatusCode::FAILURE;
  }

  m_pxlClusteringOutBuff = cl::Buffer(m_context, CL_MEM_READ_WRITE, sizeof(uint64_t) * size, NULL, &err);
  if (err != CL_SUCCESS)
  {
    ATH_MSG_ERROR("Failed to allocate output buffer for pixel clustering");
    return StatusCode::FAILURE;
  }

  m_spacepointInBuff = cl::Buffer(m_context, CL_MEM_READ_ONLY, sizeof(uint64_t) * size, NULL, &err);
  if (err != CL_SUCCESS)
  {
    ATH_MSG_ERROR("Failed to allocate input buffer for spacepoint");
    return StatusCode::FAILURE;
  }

  m_spacepointOutBuff = cl::Buffer(m_context, CL_MEM_READ_WRITE, sizeof(uint64_t) * size, NULL, &err);
  if (err != CL_SUCCESS)
  {
    ATH_MSG_ERROR("Failed to allocate output buffer for spacepoint");
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

StatusCode DataPreparationPipeline::setupKernelArgs()
{
  // Prepare kernel
  m_pixelClusteringKernel.setArg<uint>(0, 0);
  m_pixelClusteringKernel.setArg<cl::Buffer>(1, m_pxlClusteringInBuff);
  m_pixelClusteringKernel.setArg<cl::Buffer>(2, m_pxlClusteringOutBuff);

  m_spacepointKernel.setArg<cl::Buffer>(0, m_spacepointInBuff);
  m_spacepointKernel.setArg<cl::Buffer>(1, m_spacepointOutBuff);
  // Hardcoded the size for now, shouldn't need the size eventually
  m_spacepointKernel.setArg<uint>(2, 33);

  return StatusCode::SUCCESS;
}
