/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "ITkPixelDecodingAlg.h"
#include "StoreGate/ReadHandle.h"
#include "eformat/ROBFragment.h"

ITkPixelDecodingAlg::ITkPixelDecodingAlg(const std::string& name, ISvcLocator* pSvcLocator) :
  AthReentrantAlgorithm(name, pSvcLocator),
  m_packingTool("ITkPixelDataPackingTool", this),
  m_decodingTool("ITkPixelDecodingTool", this)
  //m_hitSortingTool("ITkPixelHitSortingTool", this) #commented out due to clang compilation warning, will be reintroduced in next MR
{
  
}


StatusCode ITkPixelDecodingAlg::initialize()
{
    ATH_CHECK(m_robDataProviderSvc.retrieve());

    ATH_CHECK(m_decodingTool.retrieve());

    //ATH_CHECK(m_hitSortingTool.retrieve()); #commented out due to clang compilation warning, will be reintroduced in next MR

    ATH_CHECK(m_packingTool.retrieve());

    return StatusCode::SUCCESS;
}


StatusCode ITkPixelDecodingAlg::execute(const EventContext& ctx) const
{

    //Retrieve the ROB IDs from cabling - dummy as of now
    //std::vector<uint32_t> ITkPixelSourceIDs = {0x2d27000};
    std::vector<uint32_t> ITkPixelSourceIDs = {0x00140001, 0x00770001};

    //Invoke ROBDataProviderService, fetch the concerned ROBs
    std::vector<const eformat::ROBFragment<const uint32_t*>*> ROBs;
    m_robDataProviderSvc->getROBData(ctx, ITkPixelSourceIDs, ROBs);
    
    ATH_MSG_INFO("Retrieved " << ROBs.size() << " fragments");

    //Get the payload
    for (const auto& ROB : ROBs){
        const uint32_t* payload = ROB->rod_data();
        uint32_t  length  = ROB->rod_ndata();
        ATH_MSG_DEBUG(std::hex << "Source ID: " << ROB->rob_source_id() << " L1 ID: " << ROB->rod_lvl1_id() << "\n");
        ATH_MSG_DEBUG("Length = " << length << "\n");

        for (uint32_t word = 0; word < length; word++) ATH_MSG_DEBUG("Word " << word << " = " << std::hex << "0x" << payload[word] << "\n");

    }

    return StatusCode::SUCCESS;
}