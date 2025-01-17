/*
    Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file src/IntegrationBase.cxx
 * @author zhaoyuan.cui@cern.ch
 * @date Feb. 5, 2024
 */

#include "IntegrationBase.h"
#include <fstream>

StatusCode IntegrationBase::initialize()
{
    // Perform the usual OpenCL setup here
    // Find device
    std::vector<cl::Platform> platforms;
    cl_int err = cl::Platform::get(&platforms);

    if (err == CL_SUCCESS)
    {
        ATH_MSG_INFO("Detected OpenCL platforms: " << platforms.size());
    }
    else
    {
        ATH_MSG_ERROR("Error calling cl::Platform::get. Error code: " << err);
        return StatusCode::FAILURE;
    }

    std::vector<cl::Device> allDevices;

    // Print platform information
    for (cl::Platform pf : platforms)
    {
        ATH_MSG_INFO("In initialize()");
        ATH_MSG_INFO("---");
        ATH_MSG_INFO("Platform name: " << pf.getInfo<CL_PLATFORM_NAME>());
        ATH_MSG_INFO("Platform profile: " << pf.getInfo<CL_PLATFORM_PROFILE>());
        ATH_MSG_INFO("Platform vendor: " << pf.getInfo<CL_PLATFORM_VENDOR>());

        pf.getDevices(CL_DEVICE_TYPE_ALL, &allDevices);
        ATH_MSG_INFO("There are " << allDevices.size() << " devices in this platform.");

        // Loop over all devices in the platform and see if there is an accelerator card
        // If there is an accelerator card, use the first one
        bool foundAccelerator = false;
        for(auto device : allDevices)
        {
            if(device.getInfo<CL_DEVICE_TYPE>() == CL_DEVICE_TYPE_ACCELERATOR)
            {
                ATH_MSG_INFO("Found an FPGA accelerator card in this platform");
                m_accelerator = device;
                foundAccelerator = true;
                break;
            }
        }

        // If there is no accelerator card, print error and return
        if(!foundAccelerator)
        {
            ATH_MSG_ERROR("Couldn't find an FPGA accelerator card in this platform");
            return StatusCode::FAILURE;
        }
    }

    ATH_MSG_INFO("Using FPGA accelerator card: " << m_accelerator.getInfo<CL_DEVICE_NAME>());

    // Create context
    m_context = cl::Context({m_accelerator});

    return StatusCode::SUCCESS;
}

StatusCode IntegrationBase::execute(const EventContext &ctx) const
{
    ATH_MSG_DEBUG("In execute(), event slot: "<<ctx.slot());

    return StatusCode::SUCCESS;
}

StatusCode IntegrationBase::loadProgram(const std::string& xclbin)
{
    // Open binary object in binary mode
    std::ifstream bin_file(xclbin, std::ios_base::binary);
    if (!bin_file)
    {
        ATH_MSG_ERROR("Couldn't find the xclbin file: " << xclbin);
        return StatusCode::FAILURE;
    }
    // Get the size of the binary file
    bin_file.seekg(0, bin_file.end);
    unsigned bin_size = bin_file.tellg();
    // Reset the reference point back to the beginning
    bin_file.seekg(0, bin_file.beg);
    // Create a new pointer for the binary buffer and get the set a pointer to the binary buffer
    std::vector<char> buf(bin_size);
    bin_file.read(buf.data(), bin_size);

    // Create binary object and program object
    cl_int err = 0;
    std::vector<cl_int> binaryStatus;
    cl::Program::Binaries bins{{buf.data(), bin_size}};
    m_program = cl::Program(m_context, {m_accelerator}, bins, &binaryStatus, &err);

    bin_file.close();

    if (err == CL_SUCCESS && binaryStatus.at(0) == CL_SUCCESS)
    {
        ATH_MSG_INFO("Successfully loaded xclbin file into " << m_accelerator.getInfo<CL_DEVICE_NAME>());
    }
    else
    {
        ATH_MSG_ERROR("Error loading xclbin file (" << xclbin << ") into " << m_accelerator.getInfo<CL_DEVICE_NAME>() <<". Error code: " << err);
        return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
}

StatusCode IntegrationBase::precheck(std::vector<Gaudi::Property<std::string>> inputs) const
{
    return StatusCode::SUCCESS;
}
