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

    // open the input testvector file
    std::ifstream ifs;
    ATH_MSG_DEBUG("Reading in TV");

    // Open the testvector to ifstream and check if it is open
    ifs.open(m_inputTV);
    if (!ifs.is_open())
    {
        ATH_MSG_ERROR("Error reading testvector file");
        return StatusCode::FAILURE;
    }

    uint32_t *inputTV;
    uint32_t *outputTV;

    // do 4k alignment
    posix_memalign((void **)&inputTV, 4096, sizeof(uint32_t) * 4096);
    posix_memalign((void **)&outputTV, 4096, sizeof(uint32_t) * 4096);

    // read in the file 4 bytes at a time and store it in a pointer of uint32_t
    uint32_t cache;
    int counter = 0;
    while (ifs >> std::hex >> cache)
    {
        inputTV[counter] = cache;
        counter++;

        if (counter == 4096)
        {
            break;
        }
    }
    // print the first 10 elements of the input vector
    for (int i = 0; i < 10; i++)
    {
        ATH_MSG_DEBUG("inputTV[" << std::dec << i << "] = " << std::hex << inputTV[i]);
    }

    // Close the file
    ifs.close();

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

    acc_queue.enqueueWriteBuffer(acc_inbuff, CL_TRUE, 0, sizeof(uint32_t) * 4096, inputTV, NULL, NULL);

    err = acc_queue.enqueueTask(acc_kernel);

    acc_queue.enqueueReadBuffer(acc_outbuff, CL_TRUE, 0, sizeof(uint32_t) * 4096, outputTV, NULL, NULL);

    acc_queue.finish();

    // Quick validation
    // Print the fist 10 elements of output vector
    for (int i = 0; i < 10; i++)
    {
        ATH_MSG_DEBUG("outputTV[" << std::dec << i << "] = " << std::hex << outputTV[i]);
    }

    free(inputTV);
    free(outputTV);

    return StatusCode::SUCCESS;
}