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




ITkPixelCnvTool::ITkPixelCnvTool(const std::string& type,const std::string& name,const IInterface* parent)
    : AthAlgTool(type, name, parent)
{}

/**
* @brief Retrieve helper tools
*/
StatusCode ITkPixelCnvTool::initialize(){

    //Initialize the sub-tools
    ATH_CHECK(m_hitSortingTool.retrieve());

    ATH_CHECK(m_encodingTool.retrieve());

    ATH_CHECK(m_pixelCablingKey.initialize());

    if (not m_dataRateMonTool.empty())
        ATH_CHECK(m_dataRateMonTool.retrieve());

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

    for (const auto& [onlineID, hitMap] : EventHitMaps){

        //make a copy of the onlineID
        uint64_t onID = (uint64_t)onlineID;

        //Encode the FE hits
        std::vector<uint32_t> encodedStream = m_encodingTool->encodeFE(hitMap, onID & 0x3000000000000000);

        //Build the ROD payload header. First word is size of the packet in 32bit words
        //including the header (16b), felix status (8b), swrod status (8b),
        //second is the DetectorResourceID
        uint32_t chipPayloadHeader_1 = (static_cast<uint16_t>(encodedStream.size()) + 2) << 16;
        uint32_t chipPayloadHeader_2 = onlineID.detectorResourceID();

        // passing information to the monitoring tool if initialised
        if (not m_dataRateMonTool.empty()) {
            m_dataRateMonTool->fill(onlineID.offlineModuleID(), encodedStream, hitMap);
        }

        //The ROD identifier, the sourceID, is defined in the cabling
        //and passed here from the ITkPixelHitSortingTool as part of
        //the online ID

        //Create ROD and insert the payload
        rod = fea->getRodData(onlineID.sourceID());
        rod->insert(rod->end(), {chipPayloadHeader_1, chipPayloadHeader_2});
        rod->insert(rod->end(), encodedStream.begin(), encodedStream.end());

    }

    return StatusCode::SUCCESS;
}

template StatusCode ITkPixelCnvTool::convertToByteStream<ITkPixelRDO_Container>(const ITkPixelRDO_Container* cont) const;
template StatusCode ITkPixelCnvTool::convertToByteStream<PixelRDO_Container>(const PixelRDO_Container* cont) const;
