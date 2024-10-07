/*
    Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file src/PixelClustering.cxx
 * @author zhaoyuan.cui@cern.ch
 * @date Sep. 10, 2024
 */

#include "PixelClustering.h"
#include <fstream>
#include <CL/cl_ext.h>

StatusCode PixelClustering::initialize()
{
    ATH_CHECK(IntegrationBase::precheck({m_xclbin, m_kernelName, m_inputTV, m_refTV}));
    ATH_CHECK(IntegrationBase::initialize());
    ATH_CHECK(IntegrationBase::loadProgram(m_xclbin));

    return StatusCode::SUCCESS;
}

StatusCode PixelClustering::execute(const EventContext &ctx) const
{
    ATH_MSG_DEBUG("In execute(), event slot: " << ctx.slot());

    // Make a TVHolder object
    EFTrackingFPGAIntegration::TVHolder pixelTV("PixelClustering");

    ATH_CHECK(m_testVectorTool->prepareTV(m_inputTV, pixelTV.inputTV));
    ATH_CHECK(m_testVectorTool->prepareTV(m_refTV, pixelTV.refTV));

    // Convert input vector to 32-bit unsigned integers
    std::vector<uint32_t> inputTV32;
    for (auto &elem : pixelTV.inputTV)
    {
        inputTV32.push_back(static_cast<uint32_t>(elem));
    }
    
    // print the first 10 elements of the input vector
    for (int i = 0; i < 10; i++)
    {
        ATH_MSG_DEBUG("inputTV[" << std::dec << i << "] = " << std::hex << pixelTV.inputTV[i]);
    }

    // Prepare output vector
    std::vector<uint32_t> outputTV32(4096, 0);

    // Work with the accelerator
    cl_int err = 0;

    // Allocate buffers on accelerator
    cl::Buffer acc_inbuff(m_context, CL_MEM_READ_WRITE, sizeof(uint32_t) * 4096, NULL, &err);
    cl::Buffer acc_outbuff(m_context, CL_MEM_READ_WRITE, sizeof(uint32_t) * 4096, NULL, &err);

    // Prepare kernel
    cl::Kernel acc_kernel(m_program, m_kernelName.value().data(), &err);
    acc_kernel.setArg<uint>(0, 0);
    acc_kernel.setArg<cl::Buffer>(1, acc_inbuff);
    acc_kernel.setArg<cl::Buffer>(2, acc_outbuff);

    // Make queue of commands
    cl::CommandQueue acc_queue(m_context, m_accelerator);

    acc_queue.enqueueWriteBuffer(acc_inbuff, CL_TRUE, 0, sizeof(uint32_t) * 4096, inputTV32.data(), NULL, NULL);

    err = acc_queue.enqueueTask(acc_kernel);

    acc_queue.finish();

    // The current implementation of the kernel Read/Write the same buffer
    // So the line below reads the inbuff insatead of outbuff
    acc_queue.enqueueReadBuffer(acc_inbuff, CL_TRUE, 0, sizeof(uint32_t) * 4096, outputTV32.data(), NULL, NULL);

    // Quick validation
    // Print the fist 10 elements of output vector
    for (int i = 0; i < 10; i++)
    {
        ATH_MSG_DEBUG("outputTV[" << std::dec << i << "] = " << std::hex << outputTV32[i]);
    }

    // Convert the output vector to 64-bit unsigned integers
    std::vector<uint64_t> outputTV;
    for (auto &elem : outputTV32)
    {
        outputTV.push_back(static_cast<uint64_t>(elem));
    }

    // Compare the output vector with the reference vector
    ATH_CHECK(m_testVectorTool->compare(pixelTV, outputTV));

    return StatusCode::SUCCESS;
}