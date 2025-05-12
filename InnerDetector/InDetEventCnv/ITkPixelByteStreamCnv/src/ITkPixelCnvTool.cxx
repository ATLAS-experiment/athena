/*
Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/*
* Author: Ondra Kovanda, ondrej.kovanda at cern.ch
* Date: 03/2025
* Description: Top-level tool to be called from BS converter or algorithm
*/

#include "ITkPixelCnvTool.h"
#include "ITkPixelCabling/ITkPixelOnlineId.h"
#include "InDetRawData/PixelRDO_Container.h"
#include "ITkPixelRDO_Container.h"
#include "ITkPixelHitSortingTool.h"
#include "ITkPixelEncodingTool.h"



ITkPixelCnvTool::ITkPixelCnvTool(const std::string& type,const std::string& name,const IInterface* parent)
    : AthAlgTool(type, name, parent),
    m_hitSortingTool("ITkPixelHitSortingTool", this),
    m_encodingTool("ITkPixelEncodingTool", this),
    m_byteStreamCnvSvc(this, "ByteStreamCnvSvc", "ByteStreamCnvSvc")
{}

/**
* @brief Retrieve helper tools
*/
StatusCode ITkPixelCnvTool::initialize(){

    //Initialize the sub-tools
    ATH_CHECK(m_hitSortingTool.retrieve());
    ATH_CHECK(m_encodingTool.retrieve());

    return StatusCode::SUCCESS;

}

/**
* @brief Take ITkPixelRDO_Container or PixelRDO_Container and translate it to
* bytestream
*/
template<class ContainerType>
StatusCode ITkPixelCnvTool::convertToByteStream(const ContainerType* cont) const {
    //Get the full event assembler from ByteStreamCnvSvcBase
    //SrcIdMap translates lower lvl IDs into higher lvl, e. g. ROD -> ROB, ROB -> ROS, ROS -> Det
    FullEventAssembler<SrcIdMap>* fea = 0;
    ATH_CHECK(m_byteStreamCnvSvc->getFullEventAssembler(fea, "ITkPixelRawCont"));

    fea->setDetEvtType(0x0);
    fea->setLvl1TriggerType(0x0);
    
    //Expose the buffer to be filled with the encoded data
    //Other BS converters use FullEventAssembler<SrcIdMap>::RODDATA,
    //which is just a typedef for std::vector<uint32_t>. Why the extra
    //layer of renaming?
    std::vector<uint32_t>* rod;

    //sort the RDO hits based on the FE
    const std::map<ITkPixelOnlineId, ITkPixLayout<uint16_t>> EventHitMaps = m_hitSortingTool->sortRDOHits(cont);

    //Each of the FEs in the map will have it's own ROD
    for (const auto& [onlineID, hitMap] : EventHitMaps){

        //Encode the FE hits
        std::vector<uint32_t> encodedStream = m_encodingTool->encodeFE(hitMap);

        //Create ROD and insert the payload
        rod = fea->getRodData((uint32_t)onlineID);
        rod->insert(rod->end(), encodedStream.begin(), encodedStream.end());

    }

    return StatusCode::SUCCESS;
}

template StatusCode ITkPixelCnvTool::convertToByteStream<ITkPixelRDO_Container>(const ITkPixelRDO_Container* cont) const;
template StatusCode ITkPixelCnvTool::convertToByteStream<PixelRDO_Container>(const PixelRDO_Container* cont) const;