/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDC_DETFACTORY_H
#define ZDC_DETFACTORY_H

#include "GeoModelKernel/GeoVDetectorFactory.h"
#include "GeoModelKernel/GeoDefinitions.h"
#include "AthenaBaseComps/AthMessaging.h"
//compiler needs to know ZDC_DetManager return type derives from GeoVDetectorManager
//to avoid "invalid covariant return type" warning
#include "ZDC_DetManager.h" 
#include "ZDC_ModuleBase.h"

#include <string>
#include <vector>
#include <array>

class StoreGateSvc;
class ZdcID;
class StoredMaterialManager;

class ZDC_DetFactory : public GeoVDetectorFactory,
                       public AthMessaging
{

public:
  ZDC_DetFactory(StoreGateSvc *);
  ~ZDC_DetFactory();

  virtual void create(GeoPhysVol *world) override;
  //note: baseclass returns GeoVDetectorManager *
  virtual const ZDC_DetManager *getDetectorManager() const override;
  void buildMaterials(StoredMaterialManager *materialManager);

  /*************************************************
  * The scene of the crime. Still inlined because
  * the function is just to push_back
  **************************************************/
  inline void addModule(std::unique_ptr<ZDC_ModuleBase> module) { m_modules.push_back(std::move(module) ); }
  void setTANSlot(uint iside, double width, double height, double depth, const GeoTrf::Transform3D trf, std::string name);
  
private:

  ZDC_DetManager *m_detectorManager{};
  StoreGateSvc *m_detectorStore{};
  const ZdcID *m_zdcID{};
  std::vector< std::unique_ptr<ZDC_ModuleBase> > m_modules;
  std::array<GeoTrf::Transform3D, 2> m_tanSlotTransform;
  std::array<double, 2> m_tanW, m_tanH, m_tanD;
  std::array<std::string, 2> m_tanSlotName;
};


#endif
