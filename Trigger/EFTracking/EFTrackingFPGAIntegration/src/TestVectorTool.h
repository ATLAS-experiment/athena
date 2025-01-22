/*
    Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file src/TestVectorTool.h
 * @author zhaoyuan.cui@cern.ch
 * @date Sep. 28, 2024
 * @brief Tool for test vector
 */

#ifndef EFTRACKING_FPGA_INTEGRATION__TEST_VECTOR_TOOL_H
#define EFTRACKING_FPGA_INTEGRATION__TEST_VECTOR_TOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "EFTrackingFPGAIntegration/IEFTrackingFPGAIntegrationTool.h"

#include <string>
#include <vector>

namespace EFTrackingFPGAIntegration
{
    /**
     * @struct A struct to hold the test vectors and ease the comparison
     */
    struct TVHolder
    {
        TVHolder() = default;
        TVHolder(const std::string &name) : name(name) {}
        std::string name = "";
        std::vector<uint64_t> inputTV{0};
        std::vector<uint64_t> refTV{0};
    };
}

/**
 * @class Tool for test vector
 * 
 * This tool can be used to load TV from file (.bin/.txt) and compare two TVs. 
 * This is particularly designed and used for FPGA integration development.
 */

class TestVectorTool : public extends<AthAlgTool, IEFTrackingFPGAIntegrationTool>
{
public:
    using extends::extends;

    StatusCode initialize() override;

    /**
     * @brief Prepare test vector in the form of std::vector<uint64_t>, can be either .txt or .bin
     * @param inputFile The input file name to be opened
     * @param testVector The vector of uint64_t to be filled
     */
    StatusCode prepareTV(const std::string& inputFile, std::vector<uint64_t> &testVector) const;

    /**
     * @brief Compare two TV in the form of std::vector<uint64_t>
     */
    StatusCode compare(const std::vector<uint64_t> &tv_1, const std::vector<uint64_t> &tv_2) const;

    /**
     * @brief Compare two TV in the form of TVHolder and std::vector<uint64_t>
     * @param tvHolder The TVHolder object, the refTV in the TVHolder will be compared to the output
     * @param tv_comp The vector to be compared to the refTV
     */
    StatusCode compare(const EFTrackingFPGAIntegration::TVHolder &tvHolder, const std::vector<uint64_t> &tv_comp) const;
};

#endif // EFTRACKING_FPGA_INTEGRATION__TEST_VECTOR_TOOL_H
