/*
    Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file src/OutputConversionTool.h
 * @author zhaoyuan.cui@cern.ch
 * @brief Tool for the FPGA output conversion functionality
 */

#ifndef EFTRACKING_FPGA_INTEGRATION__OUTPUT_CONVERSION_TOOL_H
#define EFTRACKING_FPGA_INTEGRATION__OUTPUT_CONVERSION_TOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "EFTrackingFPGAIntegration/IEFTrackingFPGAIntegrationTool.h"
#include "EFTrackingDataFormats.h"

namespace OutputConversion
{
    // define the FSM for the output conversion
    enum class FSM
    {
        Unknown,
        EventHeader,
        EventFooter,
        GlobalHits,
        PixelEDM,
        StripEDM,
        Error
    };
}

class OutputConversionTool : public extends<AthAlgTool, IEFTrackingFPGAIntegrationTool>
{
public:
    using extends::extends;
    StatusCode initialize() override;

    // User-level functions to decode the clusters/L2G/SpacePoints
    StatusCode decodePixelEDM(const std::vector<uint64_t> &bytestream,
                              EFTrackingDataFormats::Metadata *metadata,
                              EFTrackingDataFormats::PixelClusterAuxInput &pcAux) const;

    StatusCode decodeStripEDM(const std::vector<uint64_t> &bytestream,
                              EFTrackingDataFormats::Metadata *metadata,
                              EFTrackingDataFormats::StripClusterAuxInput &scAux) const;

    StatusCode decodeSpacePoints(const std::vector<uint64_t> &bytestream, EFTrackingDataFormats::Metadata *metadata) const;

    /**
     * @brief Decode the FPGA output based on the type. user shouldn't call this function directly
     * They should call the following user-level functions
     */
    StatusCode decodeFPGAoutput(const std::vector<uint64_t> &bytestream,
                                EFTrackingDataFormats::Metadata *metadata,
                                EFTrackingDataFormats::PixelClusterAuxInput *pcAux = nullptr,
                                EFTrackingDataFormats::StripClusterAuxInput *scAux = nullptr,
                                OutputConversion::FSM blockType = OutputConversion::FSM::Unknown) const;
};

#endif // EFTRACKING_FPGA_INTEGRATION__OUTPUT_CONVERSION_TOOL_H
