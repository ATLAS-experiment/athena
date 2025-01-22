/*
    Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file src/TestVectorTool.cxx
 * @author zhaoyuan.cui@cern.ch
 * @date Sep. 28, 2024
 */

#include "TestVectorTool.h"
#include <fstream>

StatusCode TestVectorTool::initialize()
{
    ATH_MSG_INFO("Initializing TestVectorTool tool");

    return StatusCode::SUCCESS;
}

StatusCode TestVectorTool::prepareTV(const std::string& inputFile, std::vector<uint64_t> &testVector) const
{
    ATH_MSG_DEBUG("Preparing input test vector from " << inputFile);

    // Check if the input file ends with .txt or .bin
    if (inputFile.find(".txt") == std::string::npos && inputFile.find(".bin") == std::string::npos)
    {
        ATH_MSG_ERROR("Input TV file must be either .txt or .bin");
        return StatusCode::FAILURE;
    }

    // clear the test vector before reading
    testVector.clear();

    // if .txt
    if (inputFile.find(".txt") != std::string::npos)
    {
        std::ifstream file(inputFile, std::ios::in);
        if (!file.is_open())
        {
            ATH_MSG_ERROR("Cannot open file " << inputFile);
            return StatusCode::FAILURE;
        }

        uint64_t cache;
        while (file >> std::hex >> cache)
        {
            testVector.push_back(cache);
        }

        // close the file
        file.close();
    }
    else
    {
        std::ifstream file(inputFile, std::ios::binary);
        if (!file.is_open())
        {
            ATH_MSG_ERROR("Cannot open input TV file " << inputFile);
            return StatusCode::FAILURE;
        }

        uint64_t cache;
        while (file.read(reinterpret_cast<char *>(&cache), sizeof(uint64_t)))
        {
            // Reverse the byte order
            cache = __builtin_bswap64(cache);
            testVector.push_back(cache);
        }

        // close the file
        file.close();
    }

    return StatusCode::SUCCESS;
}

StatusCode TestVectorTool::compare(const std::vector<uint64_t> &tv_1, const std::vector<uint64_t> &tv_2) const
{
    ATH_MSG_DEBUG("Comparing two test vectors");

    std::vector<uint64_t>::size_type size = -1;

    if (tv_1.size() != tv_2.size())
    {
        ATH_MSG_WARNING("The two test vectors have different sizes: " << tv_1.size() << " and " << tv_2.size());
        // Use the shorter one for comparison
        size = tv_1.size() < tv_2.size() ? tv_1.size() : tv_2.size();
    }
    else
    {
        ATH_MSG_DEBUG("The two test vectors have the same size: " << tv_1.size());
        size = tv_1.size();
    }

    bool pass = true;
    for (std::vector<uint64_t>::size_type i = 0; i < size; i++)
    {
        if (tv_1[i] != tv_2[i])
        {
            ATH_MSG_DEBUG("The two test vectors are different at index " << i);
            pass = false;
        }
    }

    if (pass)
    {
        ATH_MSG_INFO("The two test vectors are the same");
    }
    else
    {
        ATH_MSG_WARNING("The two test vectors are different!");
    }

    return StatusCode::SUCCESS;
}

StatusCode TestVectorTool::compare(const EFTrackingFPGAIntegration::TVHolder &tvHolder, const std::vector<uint64_t> &tv_comp) const
{
    ATH_MSG_INFO("Comparing the FPGA output to the reference vector for " << tvHolder.name);

    std::vector<uint64_t>::size_type size = -1;

    if (tvHolder.refTV.size() != tv_comp.size())
    {
        ATH_MSG_WARNING("The two test vectors have different sizes: " << tvHolder.refTV.size() << " and " << tv_comp.size());
        // Use the shorter one for comparison
        size = tvHolder.refTV.size() < tv_comp.size() ? tvHolder.refTV.size() : tv_comp.size();
    }
    else
    {
        ATH_MSG_DEBUG("The two test vectors have the same size: " << tvHolder.refTV.size());
        size = tvHolder.refTV.size();
    }

    bool pass = true;
    for (std::vector<uint64_t>::size_type i = 0; i < size; i++)
    {
        if (tvHolder.refTV[i] != tv_comp[i])
        {
            ATH_MSG_DEBUG("The two test vectors are different at index " << i);
            pass = false;
        }
    }

    if (pass)
    {
        ATH_MSG_INFO(tvHolder.name << " FPGA output matches the reference vector");
    }
    else
    {
        ATH_MSG_WARNING(tvHolder.name << " FPGA output does not match the reference vector");
    }

    return StatusCode::SUCCESS;
}
