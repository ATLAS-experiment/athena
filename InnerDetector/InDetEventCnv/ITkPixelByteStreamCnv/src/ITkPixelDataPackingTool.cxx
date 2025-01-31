/*
Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "ITkPixelDataPackingTool.h"

ITkPixelDataPackingTool::ITkPixelDataPackingTool(const std::string& type,const std::string& name,const IInterface* parent) : 
  AthAlgTool(type,name,parent)
{
    //not much to construct as of now
}


StatusCode ITkPixelDataPackingTool::initialize(){

    return StatusCode::SUCCESS;

}

void ITkPixelDataPackingTool::pack(const ITkPixelOnlineId *onlineID, std::vector<uint32_t> *encodedStream) const {
    // simple temporary implementation of adding a 32bit onlineID header
    encodedStream->insert(encodedStream->begin() , (uint32_t)*onlineID);
}

ITkPixelDataPackingTool::UnpackedStream ITkPixelDataPackingTool::unpack(std::vector<uint32_t> *encodedStream) const {

    UnpackedStream unpackedStream;

    unpackedStream.onlineID   = (*encodedStream)[0];
    unpackedStream.dataStream = std::vector<uint32_t>((*encodedStream).begin() + 1, (*encodedStream).end()); // this is obviously super tuned to the current encoded stream setup
    ATH_MSG_DEBUG("Unpacking event for chip " << unpackedStream.onlineID );

    return unpackedStream;
}
