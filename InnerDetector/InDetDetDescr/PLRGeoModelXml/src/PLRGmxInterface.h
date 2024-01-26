/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PLRGEOMODELXML_PLRGMXINTERFACE_H
#define PLRGEOMODELXML_PLRGMXINTERFACE_H

#include <PixelGeoModelXml/PixelGmxInterface.h>

#include <map>
#include <string>

// Most functions for this interface are derived from PixelGeoModelXml/PixelGmxInterface.h
// The functions here are similar functions to that of Pixel, but have a few PLR specific differences
// such as only loading the correct PLR sensor type and identifying the modules as InDetDD::PLR
class WaferTree;

namespace InDetDD
{

class PixelDetectorManager;
class SiCommonItems;

class PLRGmxInterface : public ITk::PixelGmxInterface
{
public:
  PLRGmxInterface(PixelDetectorManager *detectorManager,
                  SiCommonItems *commonItems,
                  WaferTree *moduleTree);

  virtual int sensorId(std::map<std::string, int> &index) const override final;
  virtual void addSensorType(const std::string& clas,
                             const std::string& typeName,
                             const std::map<std::string, std::string>& parameters) override final;
  virtual void addSensor(const std::string& typeName,
                         std::map<std::string, int> &index,
                         int sequentialId,
                         GeoVFullPhysVol *fpv) override final;

private:
  void makePLRModule(const std::string& typeName,
                     const std::map<std::string, std::string> &parameters);
};

} // namespace InDetDD

#endif // PLRGEOMODELXML_PLRGMXINTERFACE_H
