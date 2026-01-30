/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ITKPIXEL_ENCODINGALG_H
#define ITKPIXEL_ENCODINGALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "InDetRawData/PixelRDO_Container.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "GaudiKernel/ToolHandle.h"
#include "ITkPixelCnvTool.h"
#include "ITkPixelCabling/ITkPixelCablingData.h"


class ITkPixelEncodingAlg : public AthReentrantAlgorithm 
{
  public:

    ITkPixelEncodingAlg(const std::string &name, ISvcLocator *pSvcLocator);

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;

  private:

    typedef std::vector< std::vector<uint32_t >> ITkPacketCollection;
  
    SG::ReadHandleKey<PixelRDO_Container> m_pixelRDOKey{this, "PixelRDOKey", "ITkPixelRDOs", "StoreGate Key of Pixel RDOs"};

    ToolHandle<ITkPixelCnvTool> m_cnvTool{this, "PixelConversionTool", "ITkPixelCnvTool", "The conversion tool"};

};
#endif

