/*
Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "ITkPixelDecodingTool.h"

ITkPixelDecodingTool::ITkPixelDecodingTool(const std::string& type,const std::string& name,const IInterface* parent) : 
  AthAlgTool(type,name,parent)
{
    //not much to construct as of now
}

StatusCode ITkPixelDecodingTool::initialize(){
    return StatusCode::SUCCESS;
}

std::unique_ptr<ITkPixelDecodingTool::HitMap> ITkPixelDecodingTool::decodeStream(std::vector<uint32_t> *dataStream) const {
    ATH_MSG_DEBUG("Decoding event");
    ATH_MSG_DEBUG("Encoded stream size " << dataStream->size());
    std::unique_ptr<ITkPixelDecodingTool::HitMap> hitmap = std::make_unique<ITkPixelDecodingTool::HitMap>();
    
    //fill the hit map
    
    return hitmap;
}