/*
    Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file src/IntegrationBase.h
 * @author zhaoyuan.cui@cern.ch
 * @date Feb. 5, 2024
 * @brief The base class for EFTracking 2nd demonstrator integration
 */

#ifndef EFTRACKING_FPGA_INTEGRATION_INTEGRATION_BASE_H
#define EFTRACKING_FPGA_INTEGRATION_INTEGRATION_BASE_H

// Athena include
#include "AthenaBaseComps/AthReentrantAlgorithm.h"


// OpenCL settings and include for Xilinx U250 accelerator card
#define CL_HPP_CL_1_2_DEFAULT_BUILD
#define CL_HPP_TARGET_OPENCL_VERSION 120
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_ENABLE_PROGRAM_CONSTRUCTION_FROM_ARRAY_COMPATIBILITY 1
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
// OpenCL include
#include "CL/cl2.hpp"

// STL include
#include <string>
#include <vector>

/**
 * @brief The base class for the EFTracking FPGA integration development. 
 * 
 * This class contains the basic and common OpenCL setups. 
 */
class IntegrationBase : public AthReentrantAlgorithm
{
public:
    using AthReentrantAlgorithm::AthReentrantAlgorithm;

    /**
     * @brief Detect the OpenCL devices and prepare OpenCL context. 
     * 
     * This should always be called by derived classes when running on the FPGA accelerator.
    */
    virtual StatusCode initialize() override;

    /**
     * @brief Should be overriden by derived classes to perform meaningful work
    */
    virtual StatusCode execute(const EventContext &ctx) const;

    /**
     * @brief Find the xclbin file and load it into the OpenCL program object
    */
    StatusCode loadProgram(const std::string& xclbin);

    /**
     * @brief Check if the the desired Gaudi properties are set
    */
    StatusCode precheck(const std::vector<Gaudi::Property<std::string>>& inputs) const;

protected:
    // Madatory OpenCL objects that needed by derived classes
    cl::Device m_accelerator; //!< Device object for the accelerator card
    cl::Context m_context;    //!< Context object for the application
    cl::Program m_program;    //!< Program object containing the kernel
    Gaudi::Property<std::string> m_deviceBDF{this, "bdfID", "", "BDF ID of the accelerator card"}; //!< BDF ID of the accelerator card
};

#endif // EFTRACKING_FPGA_INTEGRATION_INTEGRATION_BASE_H
