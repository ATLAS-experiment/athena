/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDC_MODULEBASE_H
#define ZDC_MODULEBASE_H

#include "ZdcIdentifier/ZdcID.h"
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/GeoDefinitions.h"
#include "GeoModelInterfaces/StoredMaterialManager.h"
#include "ZdcIdentifier/ZdcID.h"

class ZDC_ModuleBase{
  public:
    ZDC_ModuleBase(){m_side = 0; m_module = -1;}
    ZDC_ModuleBase(std::string name, int side, int module)
      : m_side( side ),
      m_module( module ),
      m_name( name ),
      m_trf( GeoTrf::Transform3D() )
    {}

    ZDC_ModuleBase(ZDC_ModuleBase *right, int side, int module)
    : m_side( side ),
      m_module( module ),
      m_name( right->m_name ),
      m_trf( right->m_trf )
    {}

    virtual ~ZDC_ModuleBase() = default;

    virtual void create(GeoFullPhysVol* mother, StoredMaterialManager *materialManager, const ZdcID *zdcID) = 0;
    inline void setTransform(const GeoTrf::Transform3D trf){m_trf = trf;}
    
    inline const int& getSide() const {return m_side;}
    inline const int& getModule() const {return m_module;}
    inline const std::string& getName() const {return m_name;}
    inline const GeoTrf::Transform3D& getTransform() const {return m_trf;}

  protected:

    int m_side;
    int m_module;
    std::string m_name;
    GeoTrf::Transform3D m_trf;

};


#endif