/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDC_MODULEBASE_H
#define ZDC_MODULEBASE_H

#include "ZdcIdentifier/ZdcID.h"
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/GeoAlignableTransform.h"
#include "StoreGate/StoreGateSvc.h"
#include "ZdcIdentifier/ZdcID.h"

struct Materials;

class ZDC_ModuleBase{
  public:
    ZDC_ModuleBase(){ m_detectorStore = nullptr; m_side = 0; m_module = -1; m_zdcID = nullptr;}
    ZDC_ModuleBase(StoreGateSvc *detStore, int side, int module, const ZdcID *zdcID)
      : m_detectorStore( detStore ),
      m_side( side ),
      m_module( module ),
      m_zdcID( zdcID )
    {}

    ZDC_ModuleBase(ZDC_ModuleBase *right, int side, int module): 
      m_detectorStore(right->m_detectorStore ),
      m_side( side ),
      m_module( module ),
      m_zdcID( right->m_zdcID )
      {}

    virtual ~ZDC_ModuleBase() = default;

    virtual void create(GeoFullPhysVol* mother, GeoAlignableTransform* trf) = 0;

  protected:

    StoreGateSvc *m_detectorStore;
    int m_side;
    int m_module;
    const ZdcID *m_zdcID;

};


#endif