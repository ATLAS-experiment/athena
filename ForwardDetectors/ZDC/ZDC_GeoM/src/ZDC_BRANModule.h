/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDC_BRANMODULE_H
#define ZDC_BRANMODULE_H

#include "ZDC_ModuleBase.h"

class ZDC_BRANModule : public ZDC_ModuleBase{
  public:
    // Just use the inherited constructor
    using ZDC_ModuleBase::ZDC_ModuleBase;

    virtual ~ZDC_BRANModule() = default;

    virtual void create(GeoFullPhysVol* mother, GeoAlignableTransform* trf) override;

};


#endif