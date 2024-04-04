/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDC_ZDCMODULE_H
#define ZDC_ZDCMODULE_H

#include "ZDC_ModuleBase.h"

class ZDC_ZDCModule : public ZDC_ModuleBase{
  public:
    ZDC_ZDCModule();
    ZDC_ZDCModule(StoreGateSvc *detStore, int side, int module, const ZdcID *zdcID, int pixelStart, int pixelStop);
    ZDC_ZDCModule(ZDC_ZDCModule *right, int side, int module);

    virtual ~ZDC_ZDCModule() = default;

    virtual void create(GeoFullPhysVol* mother, GeoAlignableTransform* trf) override;

  protected:
    int m_pixelStart;
    int m_pixelStop;

};


#endif