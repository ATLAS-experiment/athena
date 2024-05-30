/*
Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*
* Author: Ondra Kovanda, ondrej.kovanda at cern.ch
* Date: 05/2024
* Description: Athena tool wrapper around the ITkPix encoder
*/

#include "ITkPixelEncodingTool.h"

ITkPixelEncodingTool::ITkPixelEncodingTool(const std::string& type,const std::string& name,const IInterface* parent) : 
  AthAlgTool(type,name,parent)
{
    //not much to construct as of now
}

StatusCode ITkPixelEncodingTool::initialize(){
    //initialize the encoder
    m_encoder = std::make_unique<ITkPixV2Encoder>();
    
    //for now, use one event per stream
    m_encoder->setEventsPerStream(1);

    return StatusCode::SUCCESS;
}

std::vector<uint32_t> ITkPixelEncodingTool::encodeFE(HitMap hitMap, uint8_t FE_id){
    //call the addToStream() method. For now assuming 1-event stream.
    //The FE_id functionality for data merging is yet to be implemented.
    m_encoder->addToStream(hitMap);
    return m_encoder->getWords();
}