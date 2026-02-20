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
        
        // Store ToT+1, reserve 0 for no hit.
        // ITkPixelOnlineId onlineID = m_cablingHelper.onlineId(rdoID); To be used when cabling helper is implemented
        // HitMap and encoder labels rows/cols from 0

        //Extract the online 'base' from cabling. Last two bits are reserved
        //for chip ID.
        ITkPixelCabling::ModuleInfo mi = cabling->onlineModuleInfo(waferID);
        //auto onlineID = (waferID.get_identifier32().get_compact() << 2 ) | chip;
        auto onlineID = (uint32_t)mi.id | chip;

        ATH_MSG_DEBUG(" Chip: " << std::hex << onlineID << std::dec << " ID: " << chip << " col: " << col << "  row: " << row << " ToT: " << tot << " eta_index = " << m_pixIdHelper->eta_index(rdoID) << " phi index = " << m_pixIdHelper->phi_index(rdoID) << " rowsPerFE = " << rowsPerFE << " colsPerFE = " << colsPerFE << "\n");

        EventHitMaps[onlineID](col, row) = tot + 1;

      };
    };
    
    return EventHitMaps;

}

template std::map<ITkPixelOnlineId, ITkPixLayout<uint16_t>> ITkPixelHitSortingTool::sortRDOHits<ITkPixelRDO_Container>(const ITkPixelRDO_Container* rdoContainer, const ITkPixelCablingData* cabling) const;
template std::map<ITkPixelOnlineId, ITkPixLayout<uint16_t>> ITkPixelHitSortingTool::sortRDOHits<PixelRDO_Container>(const PixelRDO_Container* rdoContainer, const ITkPixelCablingData* cabling) const;
/*
template<class ContainerType, class RDOType>
StatusCode ITkPixelHitSortingTool::createRDO(std::map<ITkPixelOnlineId, HitMap> &EventHitMaps, ContainerType *rdoContainer) const  {
  //this is VERY preliminary/experimental.
  //Testing purposes only at this point, will certainly change.
  //Using current ID inspiration, adapted so that no compilation
  //warnings arise. All values are dummy and no functionality is expected
  //at this point.

  typedef InDetRawDataCollection< InDetRawData > PixelRawCollection;

  for (const auto& entry : EventHitMaps) {

    // Will have to conver onlineId to offlineIdHash, for now keeping it the same
    // since we don't have cabling
    const uint offlineIdHash = (uint)entry.first; // temp
    PixelRawCollection* coll = new PixelRawCollection (offlineIdHash);

    ATH_CHECK( rdoContainer->fetchOrCreate(offlineIdHash) );//Returns null if not present -- this was used in PixelRODDecoder but compiler says it's not thread safe

    // get identifier from the hash, this is not nice
    Identifier ident = m_pixIdHelper->wafer_id(offlineIdHash);
    // set the Identifier to be nice to downstream clients
    coll->setIdentifier(ident);

    StatusCode sc = rdoContainer->addCollection(coll, offlineIdHash);
    ATH_MSG_DEBUG("Adding " << offlineIdHash);
    if (sc.isFailure()){
    ATH_MSG_ERROR("failed to add Pixel RDO collection to container" );



    const Identifier pixelId; 
    const unsigned int mToT = 0xFFFF;
    const unsigned int mBCID = 0xFFFF;
    const unsigned int mLVL1ID = 0xFFFF;
    const unsigned int mLVL1A = 0xFFFF;

    coll->push_back(new RDOType(pixelId, mToT, mBCID, mLVL1ID, mLVL1A));
    }
  }
  return StatusCode::SUCCESS;
}

template StatusCode ITkPixelHitSortingTool::createRDO<ITkPixelRDO_Container, ITkPixel1RawData>(std::map<ITkPixelOnlineId, HitMap> &EventHitMaps, ITkPixelRDO_Container *rdoContainer) const;
template StatusCode ITkPixelHitSortingTool::createRDO<PixelRDO_Container, Pixel1RawData>(std::map<ITkPixelOnlineId, HitMap> &EventHitMaps, PixelRDO_Container *rdoContainer) const;
*/
