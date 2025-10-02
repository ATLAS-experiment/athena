/*
    Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file src/Spacepoints.cxx
 * @author zhaoyuan.cui@cern.ch
 * @date Feb. 9, 2024
 */

#include "EFTrackingFPGAPipeline/Spacepoints.h"
#include <fstream>

StatusCode Spacepoints::initialize()
{
    ATH_CHECK(IntegrationBase::precheck({m_xclbin, m_kernelName, m_inputTV, m_refTV}));
    ATH_CHECK(IntegrationBase::initialize());
    ATH_CHECK(IntegrationBase::loadProgram(m_xclbin));

    return StatusCode::SUCCESS;
}

StatusCode Spacepoints::execute(const EventContext &ctx) const
{
    ATH_MSG_DEBUG("In execute(), event slot: "<<ctx.slot());

    int MAX_DATA_SIZE = 46219;

    // Prepare input test vector
    EFTrackingFPGAIntegration::TVHolder spacePointsTV("Spacepoints");
    ATH_CHECK(m_testVectorTool->prepareTV(m_inputTV, spacePointsTV.inputTV));
    ATH_CHECK(m_testVectorTool->prepareTV(m_refTV, spacePointsTV.refTV));

    // print the first 10 elements of the input vector
    for (int i = 0; i < 10; i++)
    {
        ATH_MSG_DEBUG("inputTV[" << std::dec << i << "] = " << std::hex << spacePointsTV.inputTV[i]<<std::dec);
    }

    // Work with the accelerator
    cl_int err = 0;

    // Allocate buffers on acc. card
    cl::Buffer acc_inbuff(m_context, CL_MEM_READ_ONLY, spacePointsTV.inputTV.size() * sizeof(uint64_t), NULL, &err);
    cl::Buffer acc_outbuff(m_context, CL_MEM_READ_WRITE, spacePointsTV.inputTV.size() * sizeof(uint64_t), NULL, &err);

    // Prepare kernel
    // Connect kernel to buffer before command queue
    cl::Kernel acc_kernel(m_program, m_kernelName.value().data(), &err);
    acc_kernel.setArg(0, acc_inbuff);
    acc_kernel.setArg(1, acc_outbuff);
    acc_kernel.setArg<int>(2, spacePointsTV.inputTV.size());

    // Make queue of commands
    cl::CommandQueue acc_queue(m_context, m_accelerator);

    acc_queue.enqueueWriteBuffer(acc_inbuff, CL_TRUE, 0, spacePointsTV.inputTV.size() * sizeof(uint64_t), spacePointsTV.inputTV.data(), NULL, NULL);

    // // Enqueue task
    acc_queue.enqueueTask(acc_kernel);

    acc_queue.finish();

    std::vector<uint64_t> output(spacePointsTV.inputTV.size(), 0);

    acc_queue.enqueueReadBuffer(acc_outbuff, CL_TRUE, 0, spacePointsTV.inputTV.size() * sizeof(uint64_t), output.data(), NULL, NULL);

    // Compare the output with the reference
    ATH_CHECK(m_testVectorTool->compare(spacePointsTV.refTV, output));

    return StatusCode::SUCCESS;
}
