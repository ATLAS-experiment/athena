/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDC_ZDCMODULE_H
#define ZDC_ZDCMODULE_H

#include "ZDC_ModuleBase.h"

class ZDC_ZDCModule : public ZDC_ModuleBase{
  public:
    ZDC_ZDCModule();
    ZDC_ZDCModule(const std::string& name, int side, int module, int modType);
    ZDC_ZDCModule(ZDC_ZDCModule *right, int side, int module);

    virtual ~ZDC_ZDCModule() = default;

    virtual void create(GeoFullPhysVol* mother, StoredMaterialManager *materialManager, const ZdcID *zdcID) override;

  protected:
    int m_modType;

};


#endif