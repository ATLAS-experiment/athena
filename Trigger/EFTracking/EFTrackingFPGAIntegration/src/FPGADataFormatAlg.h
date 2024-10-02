/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
   */



#ifndef EFTRACKING_FPGA_INTEGRATION_FPGADATAFORMATTER_BASE_H
#define EFTRACKING_FPGA_INTEGRATION_FPGADATAFORMATTER_BASE_H

// Athena include
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "InDetRawData/PixelRDO_Container.h"
#include "InDetRawData/SCT_RDO_Container.h"
#include "FPGADataFormatTool.h"

// STL include
#include <string>
#include <vector>

/**
 * @brief Testing alogrithms for RDO to FPGA data converted 
 */
class FPGADataFormatAlg : public AthReentrantAlgorithm
{
  public:
    using AthReentrantAlgorithm::AthReentrantAlgorithm;

    virtual StatusCode initialize() override;

    /**
     * @brief Performs the data convertsion
     */
    virtual StatusCode execute(const EventContext &ctx) const override;

  private:
    SG::ReadHandleKey<PixelRDO_Container> m_pixelRDOKey  { this, "PixelRDO", "ITkPixelRDOs" };
    SG::ReadHandleKey<SCT_RDO_Container> m_stripRDOKey  { this, "StripRDO", "ITkStripRDOs" };


    // Tool for converting the RDO into FPGA format
    ToolHandle<FPGADataFormatTool> m_FPGADataFormatTool{this, "FPGADataFormatTool", "FPGADataFormatTool", "tool to convert RDOs into FPGA data format"}; 

};

#endif // EFTRACKING_FPGA_INTEGRATION_INTEGRATION_BASE_H
