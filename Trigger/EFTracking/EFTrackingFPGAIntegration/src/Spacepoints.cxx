/*
    Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file src/Spacepoints.cxx
 * @author zhaoyuan.cui@cern.ch
 * @date Feb. 9, 2024
 */

#include "Spacepoints.h"
#include <fstream>

StatusCode Spacepoints::initialize()
{
    ATH_CHECK(IntegrationBase::precheck({m_xclbin, m_kernelName, m_inputTV, m_refTV}));
    ATH_CHECK(IntegrationBase::initialize());
    ATH_CHECK(IntegrationBase::loadProgram(m_xclbin));

    return StatusCode::SUCCESS;
}

StatusCode Spacepoints::execute()
{
    ATH_MSG_DEBUG("In execute()");

    int MAX_DATA_SIZE = 46219;

    // Read in the testvector
    std::ifstream ifs;
    ATH_MSG_DEBUG("Reading in TV");

    // Open the testvector to ifstream and check if it is open
    ifs.open(m_inputTV, std::ios::binary);
    if (!ifs.is_open())
    {
        ATH_MSG_ERROR("Error reading testvector file");
        return StatusCode::FAILURE;
    }

    // make new vector of uint64_t
    std::vector<uint64_t> inputTV;
    // a new buffer to read in the file
    uint64_t temp = 0;
    // read in the file 8 bytes at a time and store it in the vector of uint64_t
    while (ifs.good())
    {
        ifs.read(reinterpret_cast<char *>(&temp), sizeof(temp));
        if (ifs.good())
        {
            // Reverse the byte order
            temp = __builtin_bswap64(temp);
            inputTV.push_back(temp);
        }
    }

    // Close the file
    ifs.close();

    // Work with the accelerator
    cl_int err = 0;

    // Allocate buffers on acc. card
    cl::Buffer acc_inbuff(m_context, CL_MEM_READ_ONLY, inputTV.size() * sizeof(uint64_t), NULL, &err);
    cl::Buffer acc_outbuff(m_context, CL_MEM_READ_WRITE, inputTV.size() * sizeof(uint64_t), NULL, &err);

    // Make queue of commands
    cl::CommandQueue acc_queue(m_context, m_accelerator);

    acc_queue.enqueueWriteBuffer(acc_inbuff, CL_TRUE, 0, inputTV.size() * sizeof(uint64_t), inputTV.data(), NULL, NULL);

    // // Prepare kernel
    cl::Kernel acc_kernel(m_program, m_kernelName.value().data(), &err);
    acc_kernel.setArg(0, acc_inbuff);
    acc_kernel.setArg(1, acc_outbuff);
    acc_kernel.setArg<int>(2, inputTV.size());

    // // Enqueue task
    acc_queue.enqueueTask(acc_kernel);

    std::vector<uint64_t> output(MAX_DATA_SIZE);
    acc_queue.enqueueReadBuffer(acc_outbuff, CL_TRUE, 0, inputTV.size() * sizeof(uint64_t), output.data(), NULL, NULL);

    acc_queue.finish();

    return StatusCode::SUCCESS;
}
