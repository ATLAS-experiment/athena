/**
 * @file src/FPGAStripClustering.h
 * @date Jan. 22, 2025
 * @brief Class for the strip clustering kernel
 */

#ifndef EFTRACKING_FPGA_INTEGRATION_FPGASTRIPCLUSTERING_H
#define EFTRACKING_FPGA_INTEGRATION_FPGASTRIPCLUSTERING_H

// EFTracking include
#include "IntegrationBase.h"

#include "InDetRawData/SCT_RDO_Container.h"
#include "InDetRawData/PixelRDO_Container.h"
#include "FPGADataFormatTool.h"

// STL include
#include <string>

class FPGAStripClustering : public IntegrationBase
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

    SG::ReadHandleKey<SCT_RDO_Container> m_stripRDOKey{this, "StripRDO", "ITkStripRDOs", "Input data for strip clustering"}; //!< Input data key

    ToolHandle<FPGADataFormatTool> m_FPGADataFormatTool{this, "FPGADataFormatTool", "FPGADataFormatTool", "tool to convert RDOs into FPGA data format"};
};

#endif // EFTRACKING_FPGA_INTEGRATION_FPGASTRIPCLUSTERING_H