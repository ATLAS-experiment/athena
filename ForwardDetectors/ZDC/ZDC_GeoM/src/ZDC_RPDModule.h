/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDC_RPDMODULE_H
#define ZDC_RPDMODULE_H

#include "ZDC_ModuleBase.h"

class ZDC_RPDModule : public ZDC_ModuleBase {
  public:
    //Just use the inherited constructor
    using ZDC_ModuleBase::ZDC_ModuleBase;

    virtual ~ZDC_RPDModule() = default;

    virtual void create(GeoFullPhysVol* mother, GeoAlignableTransform* trf) override;

};


#endif