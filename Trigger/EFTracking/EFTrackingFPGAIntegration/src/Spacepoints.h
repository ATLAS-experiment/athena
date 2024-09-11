/*
    Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file src/Spacepoints.h
 * @author zhaoyuan.cui@cern.ch
 * @date Feb. 9, 2024
 * @brief Class for the spacepoints kernel
 */

#ifndef EFTRACKING_FPGA_INTEGRATION_SPACEPOINTS_H
#define EFTRACKING_FPGA_INTEGRATION_SPACEPOINTS_H

// EFTracking include
#include "IntegrationBase.h"

// STL include
#include <string>

class Spacepoints : public IntegrationBase
{
public:
    using IntegrationBase::IntegrationBase;
    StatusCode initialize() override;
    StatusCode execute(const EventContext &ctx) const override;

private:
    Gaudi::Property<std::string> m_xclbin{this, "xclbin", "", "xclbin path and name"}; //!< Path and name of the xclbin file
    Gaudi::Property<std::string> m_kernelName{this, "KernelName", "", "Kernel name"};  //!< Kernel name
    Gaudi::Property<std::string> m_inputTV{this, "InputTV", "", "Input TestVector"};   //!< Input TestVector
    Gaudi::Property<std::string> m_refTV{this, "RefTV", "", "Reference TestVector"};   //!< Reference TestVector
};

#endif // EFTRACKING_FPGA_INTEGRATION_SPACEPOINTS_H