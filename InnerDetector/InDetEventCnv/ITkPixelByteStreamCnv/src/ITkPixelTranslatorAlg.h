/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ITKPIXELTRANSLATORALG_H
#define ITKPIXELTRANSLATORALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "ITkPixelRDO_Container.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetRawData/PixelRDO_Container.h"


class ITkPixelTranslatorAlg : public AthReentrantAlgorithm 
{
  public:

    ITkPixelTranslatorAlg(const std::string &name, ISvcLocator *pSvcLocator):AthReentrantAlgorithm(name, pSvcLocator){};

    virtual StatusCode initialize() override;

    virtual StatusCode execute(const EventContext& ctx) const override;

  private:

    SG::ReadHandleKey<PixelRDO_Container>     m_pixelRDOKey{this,    "PixelRDOKey", "ITkPixelRDOs", "StoreGate Key of Pixel RDOs"};
    SG::WriteHandleKey<ITkPixelRDO_Container> m_itkPixelRDOKey{this,    "ITkPixelRDOKey", "ITkPixelRDOs", "StoreGate Key of ITk Pixel RDOs"};
    const PixelID* m_pixelId = 0;
};

#endif

