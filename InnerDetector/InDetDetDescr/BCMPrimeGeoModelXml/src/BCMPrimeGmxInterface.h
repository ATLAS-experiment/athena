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

class BCMPrimeGmxInterface: public GmxInterface, public AthMessaging
{
public:
  BCMPrimeGmxInterface(BCMPrimeDetectorManager* detectorManager = nullptr);

  virtual int sensorId(std::map<std::string, int> &index) const override final;
  virtual void addSensorType(const std::string& clas,
                             const std::string& typeName,
                             const std::map<std::string, std::string>& parameters) override final;

  void addSensor(const std::string& typeName,
                 std::map<std::string, int>& index,
                 int sensitiveId,
                 GeoVFullPhysVol* fpv);

  void makeBCMPrimeDiamondDesign(const std::string& typeName,
                                  const std::map<std::string, std::string>& parameters);

private:
  std::map<std::string, InDetDD::SiDetectorDesign*> m_geometryMap;
  BCMPrimeDetectorManager* m_detectorManager;
};

} // namespace InDetDD

#endif // BCMPRIMEGEOMODELXML_BCMPRIMEGMXINTERFACE_H
