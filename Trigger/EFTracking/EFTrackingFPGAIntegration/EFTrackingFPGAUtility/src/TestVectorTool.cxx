/*
    Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file src/TestVectorTool.cxx
 * @author zhaoyuan.cui@cern.ch
 * @date Sep. 28, 2024
 */

#include "EFTrackingFPGAUtility/TestVectorTool.h"
#include "EFTrackingFPGAUtility/FPGADataFormatUtilities.h"
#include "InDetMeasurementUtilities/Helpers.h"
#include <fstream>

StatusCode TestVectorTool::initialize()
{
    ATH_MSG_INFO("Initializing TestVectorTool tool");
    
    return StatusCode::SUCCESS;
}

StatusCode TestVectorTool::prepareTV(const std::string &inputFile, std::vector<uint64_t> &testVector) const
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
        ATH_MSG_DEBUG("The two test vectors are the same");
    }
    else
    {
        ATH_MSG_WARNING("The two test vectors are different!");
    }

    return StatusCode::SUCCESS;
}

StatusCode TestVectorTool::compare(const EFTrackingFPGAIntegration::TVHolder &tvHolder, const std::vector<uint64_t> &tv_comp) const
{
    ATH_MSG_DEBUG("Comparing the FPGA output to the reference vector for " << tvHolder.name);

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
        ATH_MSG_DEBUG(tvHolder.name << " FPGA output matches the reference vector");
    }
    else
    {
        ATH_MSG_WARNING(tvHolder.name << " FPGA output does not match the reference vector");
    }

    return StatusCode::SUCCESS;
}

StatusCode TestVectorTool::encodePixelL2G(const xAOD::PixelClusterContainer *pixelClusters, std::vector<uint64_t> &encodedData) const
{
    ATH_MSG_DEBUG("Encoding xAOD pixel clusters to L2G EDM TV");

    // Fill the event header
    // Event header w1
    auto header_w1 = FPGADataFormatUtilities::fill_EVT_HDR_w1(FPGADataFormatUtilities::EVT_HDR_FLAG, 1, 0, 0);
    encodedData.push_back(FPGADataFormatUtilities::get_dataformat_EVT_HDR_w1(header_w1));

    // Event header w2
    auto header_w2 = FPGADataFormatUtilities::fill_EVT_HDR_w2(242000, 0);
    encodedData.push_back(FPGADataFormatUtilities::get_dataformat_EVT_HDR_w2(header_w2));

    // Event header w3
    auto header_w3 = FPGADataFormatUtilities::fill_EVT_HDR_w3(0, 0);
    encodedData.push_back(FPGADataFormatUtilities::get_dataformat_EVT_HDR_w3(header_w3));

    // L2G conetent
    // Loop over the pixel clusters
    unsigned int isLast = 0;
    for (unsigned int i = 0; i < pixelClusters->size(); i++)
    {
        // Pixel cluster w1
        auto pixelCluster_w1 = FPGADataFormatUtilities::fill_EDM_PIXELCLUSTER_w1(FPGADataFormatUtilities::EDM_PIXELCLUSTER_FLAG,
                                                                                 pixelClusters->at(i)->identifierHash(),
                                                                                 0);
        encodedData.push_back(FPGADataFormatUtilities::get_dataformat_EDM_PIXELCLUSTER_w1(pixelCluster_w1));

     

        // Determine the size of rdo list and retrieve accordingly
        uint64_t rdoList[4] = {0, 0, 0, 0}; // Current dataformat only supports 4 RDOs

        unsigned int rdoListSize = pixelClusters->at(i)->rdoList().size();
        rdoListSize = rdoListSize > 4 ? 4 : rdoListSize; // restrict to 4 RDOs if more
        for (unsigned int j = 0; j < rdoListSize; j++)
        {
            rdoList[j] = pixelClusters->at(i)->rdoList().at(j).get_compact();
        }

        // Pixel cluster w2
        auto pixelCluster_w2 = FPGADataFormatUtilities::fill_EDM_PIXELCLUSTER_w2(rdoList[0]);
        encodedData.push_back(FPGADataFormatUtilities::get_dataformat_EDM_PIXELCLUSTER_w2(pixelCluster_w2));

        // Pixel cluster w3
        auto pixelCluster_w3 = FPGADataFormatUtilities::fill_EDM_PIXELCLUSTER_w3(rdoList[1]);
        encodedData.push_back(FPGADataFormatUtilities::get_dataformat_EDM_PIXELCLUSTER_w3(pixelCluster_w3));

        // Pixel cluster w4
        auto pixelCluster_w4 = FPGADataFormatUtilities::fill_EDM_PIXELCLUSTER_w4(rdoList[2]);
        encodedData.push_back(FPGADataFormatUtilities::get_dataformat_EDM_PIXELCLUSTER_w4(pixelCluster_w4));

        // Pixel cluster w5
        auto pixelCluster_w5 = FPGADataFormatUtilities::fill_EDM_PIXELCLUSTER_w5(rdoList[3]);
        encodedData.push_back(FPGADataFormatUtilities::get_dataformat_EDM_PIXELCLUSTER_w5(pixelCluster_w5));

        // Pixel cluster w6
        auto pixelCluster_w6 = FPGADataFormatUtilities::fill_EDM_PIXELCLUSTER_w6(pixelClusters->at(i)->localPosition<2>()(0, 0),
                                                                                 pixelClusters->at(i)->localPosition<2>()(1, 0),
                                                                                 pixelClusters->at(i)->channelsInPhi(),
                                                                                 pixelClusters->at(i)->channelsInEta(),
                                                                                 pixelClusters->at(i)->widthInEta()
                                                                                );
        encodedData.push_back(FPGADataFormatUtilities::get_dataformat_EDM_PIXELCLUSTER_w6(pixelCluster_w6));

        // Pixel cluster w7
        auto pixelCluster_w7 = FPGADataFormatUtilities::fill_EDM_PIXELCLUSTER_w7(pixelClusters->at(i)->localCovariance<2>()(0, 0),
                                                                                 pixelClusters->at(i)->localCovariance<2>()(1, 1),
                                                                                 0,
										                                         0
                                                                                 );
        encodedData.push_back(FPGADataFormatUtilities::get_dataformat_EDM_PIXELCLUSTER_w7(pixelCluster_w7));

        // Pixel cluster w8
        auto pixelCluster_w8 = FPGADataFormatUtilities::fill_EDM_PIXELCLUSTER_w8(pixelClusters->at(i)->globalPosition()[0],
                                                                                 pixelClusters->at(i)->globalPosition()[1],
                                                                                 0);
        encodedData.push_back(FPGADataFormatUtilities::get_dataformat_EDM_PIXELCLUSTER_w8(pixelCluster_w8));

        // Pixel cluster w9
        auto pixelCluster_w9 = FPGADataFormatUtilities::fill_EDM_PIXELCLUSTER_w9(pixelClusters->at(i)->identifier());
        encodedData.push_back(FPGADataFormatUtilities::get_dataformat_EDM_PIXELCLUSTER_w9(pixelCluster_w9));

        // Pixel cluster w10
        isLast = i == (pixelClusters->size() - 1) ? 1 : 0;
        auto pixelCluster_w10 = FPGADataFormatUtilities::fill_EDM_PIXELCLUSTER_w10(pixelClusters->at(i)->globalPosition()[2],
                                                                                   pixelClusters->at(i)->totalToT(),
                                                                                   isLast,
                                                                                   0);
        encodedData.push_back(FPGADataFormatUtilities::get_dataformat_EDM_PIXELCLUSTER_w10(pixelCluster_w10));
    }

    // Fill the event footer
    // Event footer w1
    auto footer_w1 = FPGADataFormatUtilities::fill_EVT_FTR_w1(FPGADataFormatUtilities::EVT_FTR_FLAG, 0, 0);
    encodedData.push_back(FPGADataFormatUtilities::get_dataformat_EVT_FTR_w1(footer_w1));

    // Event footer w2
    auto footer_w2 = FPGADataFormatUtilities::fill_EVT_FTR_w2(0);
    encodedData.push_back(FPGADataFormatUtilities::get_dataformat_EVT_FTR_w2(footer_w2));

    // Event footer w3
    auto footer_w3 = FPGADataFormatUtilities::fill_EVT_FTR_w3(encodedData.size(), 44939973);
    encodedData.push_back(FPGADataFormatUtilities::get_dataformat_EVT_FTR_w3(footer_w3));

    return StatusCode::SUCCESS;
}

StatusCode TestVectorTool::encodeStripL2G(const xAOD::StripClusterContainer *stripClusters, std::vector<uint64_t> &encodedData) const
{
    ATH_MSG_DEBUG("Encoding xAOD strip clusters to L2G EDM TV");

    // Fill the event header
    // Event header w1
    auto header_w1 = FPGADataFormatUtilities::fill_EVT_HDR_w1(FPGADataFormatUtilities::EVT_HDR_FLAG, 1, 0, 0);
    encodedData.push_back(FPGADataFormatUtilities::get_dataformat_EVT_HDR_w1(header_w1));

    // Event header w2
    auto header_w2 = FPGADataFormatUtilities::fill_EVT_HDR_w2(242000, 0);
    encodedData.push_back(FPGADataFormatUtilities::get_dataformat_EVT_HDR_w2(header_w2));

    // Event header w3
    auto header_w3 = FPGADataFormatUtilities::fill_EVT_HDR_w3(0, 0);
    encodedData.push_back(FPGADataFormatUtilities::get_dataformat_EVT_HDR_w3(header_w3));

    // L2G conetent
    // Loop over the strip clusters
    unsigned int isLast = 0;
    for (unsigned int i = 0; i < stripClusters->size(); i++)
    {
        // Strip cluster w1
        auto stripCluster_w1 = FPGADataFormatUtilities::fill_EDM_STRIPCLUSTER_w1(FPGADataFormatUtilities::EDM_STRIPCLUSTER_FLAG,
                                                                                 stripClusters->at(i)->identifierHash(),
                                                                                 0);
        encodedData.push_back(FPGADataFormatUtilities::get_dataformat_EDM_STRIPCLUSTER_w1(stripCluster_w1));

       

        // Determine the size of rdo list and retrieve accordingly
        uint64_t rdoList[4] = {0, 0, 0, 0}; // Current dataformat only supports 4 RDOs

        unsigned int rdoListSize = stripClusters->at(i)->rdoList().size();
        rdoListSize = rdoListSize > 4 ? 4 : rdoListSize; // restrict to 4 RDOs if more
        for (unsigned int j = 0; j < rdoListSize; j++)
        {
            rdoList[j] = stripClusters->at(i)->rdoList().at(j).get_compact();
        }

        // Strip cluster w3
        auto stripCluster_w2 = FPGADataFormatUtilities::fill_EDM_STRIPCLUSTER_w2(rdoList[0]);
        encodedData.push_back(FPGADataFormatUtilities::get_dataformat_EDM_STRIPCLUSTER_w2(stripCluster_w2));

        // Strip cluster w3
        auto stripCluster_w3 = FPGADataFormatUtilities::fill_EDM_STRIPCLUSTER_w3(rdoList[1]);
        encodedData.push_back(FPGADataFormatUtilities::get_dataformat_EDM_STRIPCLUSTER_w3(stripCluster_w3));

        // Strip cluster w4
        auto stripCluster_w4 = FPGADataFormatUtilities::fill_EDM_STRIPCLUSTER_w4(rdoList[2]);
        encodedData.push_back(FPGADataFormatUtilities::get_dataformat_EDM_STRIPCLUSTER_w4(stripCluster_w4));

        // Strip cluster w5
        auto stripCluster_w5 = FPGADataFormatUtilities::fill_EDM_STRIPCLUSTER_w5(rdoList[3]);
        encodedData.push_back(FPGADataFormatUtilities::get_dataformat_EDM_STRIPCLUSTER_w5(stripCluster_w5));

        // Strip cluster w6
        auto stripCluster_w6 = FPGADataFormatUtilities::fill_EDM_STRIPCLUSTER_w6(stripClusters->at(i)->localPosition<1>()(0, 0),
                                                                                 0, // Strip cluster has no local position y
                                                                                 stripClusters->at(i)->localCovariance<1>()(0, 0),
                                                                                 0);
        encodedData.push_back(FPGADataFormatUtilities::get_dataformat_EDM_STRIPCLUSTER_w6(stripCluster_w6));

        // Strip cluster w7
        auto stripCluster_w7 = FPGADataFormatUtilities::fill_EDM_STRIPCLUSTER_w7(stripClusters->at(i)->globalPosition()[0],
                                                                                 stripClusters->at(i)->globalPosition()[1],
                                                                                 stripClusters->at(i)->channelsInPhi(),
                                                                                 0);
        encodedData.push_back(FPGADataFormatUtilities::get_dataformat_EDM_STRIPCLUSTER_w7(stripCluster_w7));

         // Strip cluster w8
         auto stripCluster_w8 = FPGADataFormatUtilities::fill_EDM_STRIPCLUSTER_w8(stripClusters->at(i)->identifier());
         encodedData.push_back(FPGADataFormatUtilities::get_dataformat_EDM_STRIPCLUSTER_w8(stripCluster_w8));

        // Strip cluster w9
        isLast = i == (stripClusters->size() - 1) ? 1 : 0;
        auto stripCluster_w9 = FPGADataFormatUtilities::fill_EDM_STRIPCLUSTER_w9(stripClusters->at(i)->globalPosition()[2],
                                                                                 isLast, i, 0);
        encodedData.push_back(FPGADataFormatUtilities::get_dataformat_EDM_STRIPCLUSTER_w9(stripCluster_w9));
    }

    // Fill the event footer
    // Event footer w1
    auto footer_w1 = FPGADataFormatUtilities::fill_EVT_FTR_w1(FPGADataFormatUtilities::EVT_FTR_FLAG, 0, 0);
    encodedData.push_back(FPGADataFormatUtilities::get_dataformat_EVT_FTR_w1(footer_w1));

    // Event footer w2
    auto footer_w2 = FPGADataFormatUtilities::fill_EVT_FTR_w2(0);
    encodedData.push_back(FPGADataFormatUtilities::get_dataformat_EVT_FTR_w2(footer_w2));

    // Event footer w3
    // 44939973 in the crc field is a dummy value for now
    auto footer_w3 = FPGADataFormatUtilities::fill_EVT_FTR_w3(encodedData.size(), 44939973);
    encodedData.push_back(FPGADataFormatUtilities::get_dataformat_EVT_FTR_w3(footer_w3));

    return StatusCode::SUCCESS;
}
