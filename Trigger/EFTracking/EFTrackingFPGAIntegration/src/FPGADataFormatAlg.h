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
#include "TestVectorTool.h"
#include "OutputConversionTool.h"
#include "xAODContainerMaker.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/IChronoSvc.h"

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

    // Test vector related properties
    Gaudi::Property<std::string> m_pixelEDMRefTVPath{this, "PixelEDMRefTV", "",
                                                       "Path to pixel EDM L2G reference test vector"}; //!< Pixel L2G reference test vector
    Gaudi::Property<std::string> m_stripEDMRefTVPath{this, "StripEDMRefTV", "",
                                                       "Path to strip EDM L2G reference test vector"}; //!< Strip L2G reference test vector
    Gaudi::Property<std::string> m_spacePointRefTVPath{this, "SpacePointRefTV", "",
                                                        "Path to space point reference test vector"}; //!< Space point reference test vector

    // Tool for converting the RDO into FPGA format
    ToolHandle<FPGADataFormatTool> m_FPGADataFormatTool{this, "FPGADataFormatTool", "FPGADataFormatTool", "tool to convert RDOs into FPGA data format"}; 

    // Tool for reading in TV
    ToolHandle<TestVectorTool> m_testVectorTool{this, "TestVectorTool", "TestVectorTool", "tool to read in test vector"};

    // Tool for output conversion
    ToolHandle<OutputConversionTool> m_outputConversionTool{this, "OutputConversionTool", "OutputConversionTool", "tool for output conversion"};

    // Tool for making xAOD containers
    ToolHandle<xAODContainerMaker> m_xAODContainerMaker{this, "xAODContainerMaker", "xAODContainerMaker", "tool for making xAOD containers"};

    // Chrono service
    ServiceHandle<IChronoStatSvc> m_chronoSvc{"ChronoStatSvc", name()};
};

#endif // EFTRACKING_FPGA_INTEGRATION_INTEGRATION_BASE_H
