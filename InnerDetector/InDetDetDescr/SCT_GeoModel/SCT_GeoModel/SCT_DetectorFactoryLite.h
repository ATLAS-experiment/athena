/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef SCT_GEOMODEL_SCT_DETECTORFACTORYLITE_H
#define SCT_GEOMODEL_SCT_DETECTORFACTORYLITE_H

#include "SCT_ReadoutGeometry/SCT_DetectorManager.h"
#include "SCT_GeoModel/SCT_GeometryManager.h"

#include "InDetGeoModelUtils/InDetDetectorFactoryBase.h" 
#include "ReadoutGeometryBase/InDetDD_Defs.h"
#include "CxxUtils/checker_macros.h"
#include "AthenaBaseComps/AthMessaging.h"

#include <memory>
#include <map>
#include <string>

class GeoPhysVol;
class GeoFullPhysVol;
class GeoAlignableTransform;

class SCT_DataBase;
class SCT_GeoModelAthenaComps;
class SCT_MaterialManager;
class SCT_Options;

namespace GeoModelIO {
  class ReadGeoModel;
}

class SCT_DetectorFactoryLite : public InDetDD::DetectorFactoryBase
{ 
  
 public: 
  // Constructor
  SCT_DetectorFactoryLite(GeoModelIO::ReadGeoModel *sqliteReader,SCT_GeoModelAthenaComps * athenaComps,
		      const SCT_Options & options);

  // Destructor
  virtual ~SCT_DetectorFactoryLite() = default;

  // Creation of geometry:
  virtual void create(GeoPhysVol *world) override;

  // Access to the results: 
  virtual const InDetDD::SCT_DetectorManager * getDetectorManager() const override;

 private: 
  // Copy and assignments operations illegal and so are made private
  SCT_DetectorFactoryLite(const SCT_DetectorFactoryLite &right) = delete;
  const SCT_DetectorFactoryLite & operator=(const SCT_DetectorFactoryLite &right) = delete;

  // private member data:
  GeoModelIO::ReadGeoModel    *m_sqliteReader{nullptr};
  InDetDD::SCT_DetectorManager *m_detectorManager{nullptr};
  std::unique_ptr<SCT_GeometryManager> m_geometryManager;
  std::unique_ptr<SCT_DataBase> m_db;
  bool m_useDynamicAlignFolders{false};

  using FPVMap = std::map<std::string, GeoFullPhysVol*>;
  using AXFMap = std::map<std::string, GeoAlignableTransform*>;
  
  std::shared_ptr<FPVMap>  m_mapFPV;
  std::shared_ptr<AXFMap>  m_mapAXF;
}; 
 
#endif 
 
