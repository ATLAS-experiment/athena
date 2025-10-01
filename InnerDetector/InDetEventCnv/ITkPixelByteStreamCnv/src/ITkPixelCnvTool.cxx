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
#include "ITkPixelCabling/ITkPixelCablingData.h"




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

    ATH_CHECK(m_pixelCablingKey.initialize());

    return StatusCode::SUCCESS;

}

/**
* @brief Take ITkPixelRDO_Container or PixelRDO_Container and translate it to
* bytestream
*/
template<class ContainerType>
StatusCode ITkPixelCnvTool::convertToByteStream(const ContainerType* cont) const {
    
    //Get the cabling
    SG::ReadCondHandle<ITkPixelCablingData> cablingData(m_pixelCablingKey);
    const ITkPixelCablingData* cabling = *cablingData;

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
    const std::map<ITkPixelOnlineId, ITkPixLayout<uint16_t>> EventHitMaps = m_hitSortingTool->sortRDOHits(cont, cabling);

    int nRODs = 0;

    for (const auto& [onlineID, hitMap] : EventHitMaps){

        //make a copy of the onlineID
        uint32_t onID = (uint32_t)onlineID;

        //Encode the FE hits
        std::vector<uint32_t> encodedStream = m_encodingTool->encodeFE(hitMap, onID & 0x3);

        //Is this a merged quad? If so, collapse the chip ID in the online identifier to 00
        //we can get the information from cabling's online -> modlue Info map.
        ITkPixelCabling::ModuleInfo mi = cabling->offlineModuleInfo((onID >> 2) << 2);
        if (mi.type == ITkPixelCabling::ModuleType::MergedQuad) onID = (onID >> 2) << 2;

        //At this point, the ROD identifier will be labelled with lowest two bits 00 for single
        //chips, chip 00 in unmerged quads and for entire merged quads, and with
        //the rest up to 0x3 for the remaining 3 chips in unmerged quads. The higher bits are
        //the online 'base'. We need this because of the unmerged quads, where one
        //offline entity maps on 4 online entities.
        

        //Create ROD and insert the payload
        rod = fea->getRodData(onID);
        if (rod->size() == 0) nRODs++;
        rod->insert(rod->end(), encodedStream.begin(), encodedStream.end());

    }
    std::cout << "nRODs = " << nRODs << "\n";

    return StatusCode::SUCCESS;
}

template StatusCode ITkPixelCnvTool::convertToByteStream<ITkPixelRDO_Container>(const ITkPixelRDO_Container* cont) const;
template StatusCode ITkPixelCnvTool::convertToByteStream<PixelRDO_Container>(const PixelRDO_Container* cont) const;