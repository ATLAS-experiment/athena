/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "ITkPixelTranslatorAlg.h"
#include "ITkPixel1RawData.h"
#include <memory>


StatusCode ITkPixelTranslatorAlg::initialize(){
    ATH_CHECK(m_pixelRDOKey.initialize());
    ATH_CHECK(m_itkPixelRDOKey.initialize());
    ATH_CHECK( detStore()->retrieve(m_pixelId, "PixelID") );
    return StatusCode::SUCCESS;

};

/**
* @brief utility to translate InDet RDO into ITk RDO
* and record it back to SG
*/
StatusCode ITkPixelTranslatorAlg::execute(const EventContext& ctx) const {
    SG::ReadHandle<PixelRDO_Container> rdoContainerHandle(m_pixelRDOKey, ctx);
    std::unique_ptr<ITkPixelRDO_Container> itkpixelrdocontainer = std::make_unique<ITkPixelRDO_Container>(m_pixelId->wafer_hash_max());
    for (const auto& coll : *rdoContainerHandle){
        std::unique_ptr<InDetRawDataCollection< ITkPixelRDORawData >> itkColl = std::make_unique<InDetRawDataCollection< ITkPixelRDORawData >>(coll->identifyHash());
        for (const auto& rdo : *coll){
            std::unique_ptr<ITkPixel1RawData> itkrdo = std::make_unique<ITkPixel1RawData>(rdo->identify(), rdo->getWord());
            itkColl->push_back(itkrdo.release());
        }
        IdentifierHash hash = itkColl->identifyHash();
        ATH_CHECK(itkpixelrdocontainer->addCollection(itkColl.release(), hash));
    }
    SG::WriteHandle<ITkPixelRDO_Container> itkRDOContainerHandle(m_itkPixelRDOKey, ctx);
    ATH_CHECK(itkRDOContainerHandle.record(std::move(itkpixelrdocontainer)));
    return StatusCode::SUCCESS;
}
