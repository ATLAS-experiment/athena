/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ITKPIXEL_ENCODINGALG_H
#define ITKPIXEL_ENCODINGALG_Hi

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "InDetRawData/PixelRDO_Container.h"
#include "StoreGate/ReadHandleKey.h"

namespace InDetDD {
  class PixelDetectorManager;
}

class PixelID;


class ITkPixelEncodingAlg : public AthReentrantAlgorithm 
{
  public:
    ITkPixelEncodingAlg(const std::string &name, ISvcLocator *pSvcLocator);
    ~ITkPixelEncodingAlg(){}

    virtual StatusCode initialize();
    virtual StatusCode execute (const EventContext& ctx) const;

  private:

    enum Region {
      INVALID_REGION=-1, BARREL, ENDCAP, N_REGIONS
    };

    SG::ReadHandleKey<PixelRDO_Container> m_pixelRDOKey{this,"PixelRDOKey","ITkPixelRDOs","StoreGate Key of Pixel RDOs"};
    const InDetDD::PixelDetectorManager*           m_pixelManager;    
    const PixelID*                                 m_pixIdHelper;  

    static constexpr float s_pitch50x50=0.050;
    
};
#endif

