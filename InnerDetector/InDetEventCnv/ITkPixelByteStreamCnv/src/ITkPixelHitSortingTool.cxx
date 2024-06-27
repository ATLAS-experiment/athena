/*
Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "ITkPixelHitSortingTool.h"
#include "InDetIdentifier/PixelID.h"



ITkPixelHitSortingTool::ITkPixelHitSortingTool(const std::string& type,const std::string& name,const IInterface* parent) : 
  AthAlgTool(type,name,parent)
{
    //not much to construct as of now
}


StatusCode ITkPixelHitSortingTool::initialize(){
    ATH_CHECK(m_pixelReadout.retrieve());
    if (!detStore()->retrieve(m_pixIdHelper, "PixelID").isSuccess()) {
        ATH_MSG_FATAL("Unable to retrieve PixelID helper");
        return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
}

StatusCode ITkPixelHitSortingTool::sortRDOHits(SG::ReadHandle<PixelRDO_Container> &rdoContainer) const {
    PixelRDO_Container::const_iterator rdoCollections      = rdoContainer->begin();
    PixelRDO_Container::const_iterator rdoCollectionsEnd   = rdoContainer->end();

    for(; rdoCollections!=rdoCollectionsEnd; ++rdoCollections){
      const InDetRawDataCollection<PixelRDORawData>* RDO_Collection(*rdoCollections);

      for(const auto *const rdo : *RDO_Collection) {
        const Identifier rdoID = rdo->identify();
        const Identifier wafferID = m_pixIdHelper->wafer_id(rdoID);

        int pixPhiIx = m_pixIdHelper->phi_index(rdoID);
        int pixEtaIx = m_pixIdHelper->eta_index(rdoID);

        const int tot = rdo->getToT();
        uint32_t chip = m_pixelReadout->getFE( rdoID, wafferID);
        uint32_t col =  m_pixelReadout->getColumn( rdoID, wafferID);
        uint32_t row =  m_pixelReadout->getRow( rdoID, wafferID);

        ATH_MSG_INFO("ID: " + rdoID.getString() + " IDoff: " + wafferID.getString() + " Chip: "+ std::to_string(chip) + "  ToT: " + std::to_string(tot) + " pixEtaIx: " +  std::to_string(pixEtaIx) + " col: " +  std::to_string(col)+ "  pixPhiIx: " + std::to_string(pixPhiIx) + "  row: " + std::to_string(row));
      };
    };



    return StatusCode::SUCCESS;
}