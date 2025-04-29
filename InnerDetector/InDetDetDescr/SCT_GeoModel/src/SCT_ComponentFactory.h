/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef SCT_GEOMODEL_SCT_COMPONENTFACTORY_H
#define SCT_GEOMODEL_SCT_COMPONENTFACTORY_H

#include "SCT_Identifier.h"
#include <string>
#include <map>
#include <memory>

namespace InDetDD{class SCT_DetectorManager;}
class SCT_GeometryManager;
class SCT_MaterialManager;

class GeoLogVol;
class GeoVPhysVol;
class GeoFullPhysVol;
class GeoAlignableTransform;

namespace GeoModelIO {
  class ReadGeoModel;
}

class SCT_ComponentFactory
{

public:
  SCT_ComponentFactory(const std::string & name,
                       InDetDD::SCT_DetectorManager* detectorManager,
                       SCT_GeometryManager* geometryManager,
                       SCT_MaterialManager* materials);

  const std::string & getName() const {return m_name;}

  // utility function to covert int to string
  std::string intToString(int i) const;

protected: 
  InDetDD::SCT_DetectorManager* m_detectorManager;
  SCT_GeometryManager* m_geometryManager;
  SCT_MaterialManager* m_materials;

  double epsilon() const;
  virtual ~SCT_ComponentFactory();

private:
  std::string m_name;
  static const double s_epsilon;

};


class SCT_SharedComponentFactory : public SCT_ComponentFactory
{

public:
  SCT_SharedComponentFactory(const std::string & name,
                             InDetDD::SCT_DetectorManager* detectorManager,
                             SCT_GeometryManager* geometryManager,
                             SCT_MaterialManager* materials=nullptr) :
    SCT_ComponentFactory(name, detectorManager, geometryManager, materials),
    m_physVolume(nullptr)
  {};
  
  GeoVPhysVol * getVolume() {return  m_physVolume;}

protected:
  GeoVPhysVol * m_physVolume;
  virtual GeoVPhysVol * build() = 0;

};

class SCT_UniqueComponentFactory : public SCT_ComponentFactory
{

public:
  SCT_UniqueComponentFactory(const std::string & name,
                             InDetDD::SCT_DetectorManager* detectorManager,
                             SCT_GeometryManager* geometryManager,
                             SCT_MaterialManager* materials=nullptr,
                             GeoModelIO::ReadGeoModel* sqliteReader=nullptr,
                             std::shared_ptr<std::map<std::string, GeoFullPhysVol*>>        mapFPV=nullptr,
                             std::shared_ptr<std::map<std::string, GeoAlignableTransform*>> mapAX=nullptr);

  virtual GeoVPhysVol * build(SCT_Identifier id) = 0;

protected:
  const GeoLogVol * m_logVolume;
  GeoModelIO::ReadGeoModel* m_sqliteReader;

  virtual const GeoLogVol * preBuild() = 0;

  std::shared_ptr<std::map<std::string, GeoFullPhysVol*>>        m_mapFPV;
  std::shared_ptr<std::map<std::string, GeoAlignableTransform*>> m_mapAX;

};

#endif // SCT_GEOMODEL_SCT_COMPONENTFACTORY_H
