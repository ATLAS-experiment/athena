/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ITKPIXEL_ENCODINGALG_H
#define ITKPIXEL_ENCODINGALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "InDetRawData/PixelRDO_Container.h"
#include "StoreGate/ReadHandleKey.h"
#include "ITkPixelHitSortingTool.h"
#include "GaudiKernel/ToolHandle.h"

namespace InDetDD {
  class PixelDetectorManager;
}

class PixelID;


class ITkPixelEncodingAlg : public AthReentrantAlgorithm 
{
  public:

    ITkPixelEncodingAlg(const std::string &name, ISvcLocator *pSvcLocator);

    virtual StatusCode initialize() override;
    virtual StatusCode execute (const EventContext& ctx) const override;

  private:

    SG::ReadHandleKey<PixelRDO_Container> m_pixelRDOKey{this, "PixelRDOKey", "ITkPixelRDOs", "StoreGate Key of Pixel RDOs"};

    static constexpr float s_pitch50x50=0.050;

    ToolHandle<ITkPixelHitSortingTool> m_hitSortingTool;


};
#endif

