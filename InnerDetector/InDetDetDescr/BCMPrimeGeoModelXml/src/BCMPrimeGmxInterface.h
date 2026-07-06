/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BCMPRIMEGEOMODELXML_BCMPRIMEGMXINTERFACE_H
#define BCMPRIMEGEOMODELXML_BCMPRIMEGMXINTERFACE_H

#include <AthenaBaseComps/AthMessaging.h>
#include <GeoModelXml/GmxInterface.h>

#include <map>
#include <InDetReadoutGeometry/SiDetectorDesign.h>
#include <InDetReadoutGeometry/SiDetectorElement.h>

namespace InDetDD
{

class BCMPrimeDetectorManager;
class SiCommonItems;

class BCMPrimeGmxInterface: public GmxInterface, public AthMessaging
{
public:
  BCMPrimeGmxInterface(BCMPrimeDetectorManager* detectorManager = nullptr,
                       SiCommonItems* commonItems = nullptr);

  virtual int sensorId(std::map<std::string, int> &index) const override final;
  virtual void addSensorType(const std::string& clas,
                             const std::string& typeName,
                             const std::map<std::string, std::string>& parameters) override final;

  virtual void addSensor(const std::string& typeName,
                         std::map<std::string, int>& index,
                         int sensitiveId,
                         GeoVFullPhysVol* fpv) override;

  void makeBCMPrimeDiamondDesign(const std::string& typeName,
                                  const std::map<std::string, std::string>& parameters);

private:
  std::map<std::string, const InDetDD::SiDetectorDesign*> m_geometryMap;
  BCMPrimeDetectorManager* m_detectorManager{};
  SiCommonItems* m_commonItems{};
};

} // namespace InDetDD

#endif // BCMPRIMEGEOMODELXML_BCMPRIMEGMXINTERFACE_H
