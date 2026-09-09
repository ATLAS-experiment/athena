/*
Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "ITkPixelHitSortingTool.h"
#include "InDetIdentifier/PixelID.h"
#include "PixelReadoutGeometry/PixelDetectorManager.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include "InDetRawData/PixelRDORawData.h"
#include "InDetRawData/Pixel1RawData.h"
#include "ITkPixel1RawData.h"
#include "InDetRawData/PixelRDO_Container.h"
#include "ITkPixelRDO_Container.h"
#include "ITkPixelCabling/ITkPixelCablingData.h"


#include <map>

ITkPixelHitSortingTool::ITkPixelHitSortingTool(const std::string& type,const std::string& name,const IInterface* parent) : 
  AthAlgTool(type,name,parent)
{
    //not much to construct as of now
}


StatusCode ITkPixelHitSortingTool::initialize(){
    ATH_CHECK(m_pixelReadout.retrieve());
    ATH_CHECK(detStore()->retrieve(m_pixIdHelper, "PixelID"));
    ATH_CHECK(detStore()->retrieve(m_detManager, "ITkPixel"));
    return StatusCode::SUCCESS;
}

template<class ContainerType>
std::map<ITkPixelOnlineId, ITkPixLayout<uint16_t>> ITkPixelHitSortingTool::sortRDOHits(const ContainerType* rdoContainer, const ITkPixelCablingData* cabling) const {

    std::map<ITkPixelOnlineId, ITkPixLayout<uint16_t>> EventHitMaps;

    for(const auto& RDO_Collection : *rdoContainer){

      for(const auto *const rdo : *RDO_Collection) {

        const Identifier rdoID = rdo->identify();
        const Identifier waferID = m_pixIdHelper->wafer_id(rdoID);

        const uint16_t tot  = rdo->getToT();

        //This part will see changes. The PixelReadoutManager will be
        //retired, and the chip <-> module translation will happen likely
        //in PixelModuleDesign. Also, figuring out whether we're dealing
        //with 25x100 or R0 modules can be greatly simplified.
        //---------->
        const uint32_t chip = m_pixelReadout->getFE( rdoID, waferID);
        uint32_t col  = m_pixelReadout->getColumn( rdoID, waferID);
        uint32_t row  = m_pixelReadout->getRow( rdoID, waferID);
        
        //find out if we're dealing with 25x100 sensors
        const InDetDD::SiDetectorElement *element = m_detManager->getDetectorElement(waferID);
        const InDetDD::PixelModuleDesign *p_design = static_cast<const InDetDD::PixelModuleDesign *>(&element->design());
        const uint nChips = p_design->numberOfCircuits();
        const uint rowsPerFE =  p_design->rowsPerCircuit();
        const uint colsPerFE =  p_design->columnsPerCircuit();
        bool is25x100 = rowsPerFE == 768 && colsPerFE == 200;
        ATH_MSG_DEBUG("Module specs: nChips = " << nChips << ", rows per FE = " << rowsPerFE << " cols per FE = " << colsPerFE);
        //<----------        
        
        if (is25x100){
            //The bonding pattern as understood at the time of writing this code is
            //that odd sensor rows (with even indices if numbered from 0) are bonded to the left and even (= odd indices) to the right.
            col = 2 * col + (row + 1)% 2;
            row = row / 2;
            ATH_MSG_DEBUG("Adding hit from 25x100 pixel");
        } 
        else if (colsPerFE == 384 && rowsPerFE == 400){
            //R0 EC modules are 90 degrees rotated, the readout manager doesn't know about it
            std::swap(col, row);
            ATH_MSG_DEBUG("Rotated chip - swapping col and row");
        }
        
        //Store ToT+1, reserve 0 for no hit.
        //HitMap and encoder labels rows/cols from 0

        //Data from a FE are identified by "DetectorResourceID", which is part
        //of the header delimiting individual FE payloads within a ROD fragment
        //payload. The DetectorResourceID is detector-defined. For pixels, it
        //has an 'online part', containing chipID and information useful for decoding
        //(chipID on/off). Then there's an offline-specific part
        //composed - for convenience - of (offline) module ID. We can mask out
        //the auxiliary online info to get the mapping from FE -> online ID
        //  offlineDetectorResourceID = chipID [2 MSB] | module ID [24 LSB]
        //Furthermore, we need to know which ROB fragment the data will go to,
        //hence we need the 'sourceID' (in eformat vocabulary), which identifies
        //the DMA buffer on the FELIX machine that received the data. This is
        //all provided by the cabling package, which gives
        //  offlineDetectorResourceID -> {DetectorResourceID, sourceID}
        //mapping.

        //ITkPixelCabling::ModuleInfo mi = cabling->onlineModuleInfo(waferID);
        uint32_t offlineDetectorResourceID = (waferID.get_identifier32().get_compact() >> 8) | (chip << 30);
       
        ITkPixelOnlineId onlineID = cabling->onlineId(offlineDetectorResourceID);

        //Found a valid ID?
        if (onlineID.sourceID() == 0xFFFFFFFF){
            ATH_MSG_WARNING("Cabling not found for module bec " << std::dec << m_pixIdHelper->barrel_ec(waferID) << " ld " << m_pixIdHelper->layer_disk(waferID) << " eta " << m_pixIdHelper->eta_module(waferID) << " phi " << m_pixIdHelper->phi_module(waferID) << ", proceeding with a dummy value");
            onlineID = ITkPixelOnlineId(0x00170000 | (offlineDetectorResourceID & 0x0000FFFF), offlineDetectorResourceID | 0xF0000000);
        }

        ATH_MSG_DEBUG(" Chip: " << std::hex << onlineID << std::dec << " ID: " << chip << " col: " << col << "  row: " << row << " ToT: " << tot << " eta_index = " << m_pixIdHelper->eta_index(rdoID) << " phi index = " << m_pixIdHelper->phi_index(rdoID) << " rowsPerFE = " << rowsPerFE << " colsPerFE = " << colsPerFE << "\n");
        
        EventHitMaps[onlineID](col, row) = tot + 1;

      };
    };

    return EventHitMaps;

}

template std::map<ITkPixelOnlineId, ITkPixLayout<uint16_t>> ITkPixelHitSortingTool::sortRDOHits<ITkPixelRDO_Container>(const ITkPixelRDO_Container* rdoContainer, const ITkPixelCablingData* cabling) const;
template std::map<ITkPixelOnlineId, ITkPixLayout<uint16_t>> ITkPixelHitSortingTool::sortRDOHits<PixelRDO_Container>(const PixelRDO_Container* rdoContainer, const ITkPixelCablingData* cabling) const;