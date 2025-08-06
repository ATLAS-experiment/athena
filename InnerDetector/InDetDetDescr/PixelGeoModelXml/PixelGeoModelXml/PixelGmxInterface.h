/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PIXELGEOMODELXML_PIXELGMXINTERFACE_H
#define PIXELGEOMODELXML_PIXELGMXINTERFACE_H

#include <AthenaBaseComps/AthMessaging.h>
#include <GeoModelXml/GmxInterface.h>

#include <map>
#include <string>

class IRDBAccessSvc;
class WaferTree;

namespace GeoModelIO{
  class ReadGeoModel;
}

namespace InDetDD {

class PixelDetectorManager;
class PixelDiodeTree;
class SiCommonItems;
class SiDetectorDesign;

namespace ITk
{
class PixelGmxInterface : public GmxInterface, public AthMessaging
{
public:
  PixelGmxInterface(PixelDetectorManager *detectorManager,
                    SiCommonItems *commonItems,
                    WaferTree *moduleTree);

  virtual int sensorId(std::map<std::string, int> &index) const override;
  virtual void addSensorType(const std::string& clas,
                             const std::string& typeName,
                             const std::map<std::string, std::string>& parameters) override;
  virtual void addSensor(const std::string& typeName,
                         std::map<std::string, int> &index,
                         int sequentialId,
                         GeoVFullPhysVol *fpv) override;
  virtual void addAlignable(int level,
                            std::map<std::string, int> &index,
                            GeoVFullPhysVol *fpv,
                            GeoAlignableTransform *transform) override final;

   void buildReadoutGeometryFromSqlite(IRDBAccessSvc * rdbAccessSvc, GeoModelIO::ReadGeoModel* sqlreader);

protected:
  std::map<std::string, int> m_geometryMap;

  void makePixelModule(const std::string& typeName,
                       const std::map<std::string, std::string> &parameters);

  PixelDetectorManager *m_detectorManager{};
  SiCommonItems *m_commonItems{};
  WaferTree *m_moduleTree{};
};

} // namespace ITk
} // namespace InDetDD

#endif // PIXELGEOMODELXML_PIXELGMXINTERFACE_H
