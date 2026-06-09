/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "BCMPrimeGmxInterface.h"

#include "BCMPrimeReadoutGeometry/BCMPrimeDetectorManager.h"
#include "BCMPrimeReadoutGeometry/BCMPrimeDiamondDesign.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "InDetSimEvent/SiHitIdHelper.h"
#include "GeoModelKernel/GeoFullPhysVol.h"

namespace InDetDD
{

  BCMPrimeGmxInterface::BCMPrimeGmxInterface(BCMPrimeDetectorManager* detectorManager)
  : AthMessaging("BCMPrimeGmxInterface"), m_detectorManager(detectorManager)
{   
    ATH_MSG_INFO("PEDRO PEDRO BCMPrimeGmxInterface constructed");
    ATH_MSG_INFO("PEDRO PEDRO m_detectorManager = " << m_detectorManager);
}

int BCMPrimeGmxInterface::sensorId(std::map<std::string, int> &index) const
{
  // Return the Simulation HitID (nothing to do with "ATLAS Identifiers" aka "Offline Identifiers")
  int hitIdOfModule = SiHitIdHelper::GetHelper()->buildHitId(0, 0, index["diamond_number"], index["module_number"], 0, 0);

  ATH_MSG_INFO("Index list: " << index["diamond_number"] << " " << index["module_number"]);
  ATH_MSG_INFO("hitIdOfModule = " << std::hex << hitIdOfModule << std::dec);
  ATH_MSG_INFO(" dia = " << SiHitIdHelper::GetHelper()->getLayerDisk(hitIdOfModule) <<
                " mod = " << SiHitIdHelper::GetHelper()->getEtaModule(hitIdOfModule));
  return hitIdOfModule;
}

void BCMPrimeGmxInterface::addSensorType(const std::string& clas,
                                        const std::string& typeName,
                                        const std::map<std::string, std::string>& parameters)
{
  ATH_MSG_INFO("addSensorType called for class " << clas << ", type " << typeName);
  makeBCMPrimeDiamondDesign(typeName, parameters);
}

void BCMPrimeGmxInterface::addSensor(
    const std::string& typeName,
    std::map<std::string, int> &index,
    int /*sensitiveId*/,
    GeoVFullPhysVol* fpv)
{
    ATH_MSG_INFO("BCMPrime addSensor called for " << typeName);
    for (const auto& [k, v] : index) {
        ATH_MSG_INFO("  index[" << k << "] = " << v);
    }
    // Look up the design for this sensor type
    auto it = m_geometryMap.find(typeName);
    if (it == m_geometryMap.end()) {
        ATH_MSG_WARNING("No design found for sensor type " << typeName);
        return;
    }

    InDetDD::SiDetectorDesign* design = it->second;

    // For now we do not create full SiDetectorElement instances because
    // they require Identifier and SiCommonItems. Store the fact that a
    // sensor instance exists and the design is available.
    ATH_MSG_INFO("BCMPrime USING design for sensor type " << typeName);
    ATH_MSG_DEBUG("BCMPrime sensor instance: type=" << typeName
                  << " diamond=" << index["diamond_number"]
                  << " module=" << index["module_number"]);
    // Optionally register the physical volume as a tree top so GeoModel
    // alignment machinery can still see it.
    if (m_detectorManager && fpv) {
        m_detectorManager->addTreeTop(fpv);
    }
}

void BCMPrimeGmxInterface::makeBCMPrimeDiamondDesign(const std::string& typeName,
                                                      const std::map<std::string, std::string>& parameters)
{
    ATH_MSG_DEBUG("makeBCMPrimeDiamondDesign for type " << typeName);

    // Check if we already created this design
    if (m_geometryMap.find(typeName) != m_geometryMap.end()) {
        ATH_MSG_DEBUG("Design " << typeName << " already exists");
        return;
    }

    // Parse parameters from ITKLayouts (bcm_full.xml)
    double sizeX = 10.0;        // Default: 10 mm
    double sizeY = 10.0;        // Default: 10 mm
    double thickness = 0.5;     // Default: 0.5 mm diamond

    auto sizeX_it = parameters.find("sizeX");
    if (sizeX_it != parameters.end()) {
        try {
            sizeX = std::stod(sizeX_it->second);
        } catch (...) {
            ATH_MSG_WARNING("Could not parse sizeX parameter");
        }
    }

    auto sizeY_it = parameters.find("sizeY");
    if (sizeY_it != parameters.end()) {
        try {
            sizeY = std::stod(sizeY_it->second);
        } catch (...) {
            ATH_MSG_WARNING("Could not parse sizeY parameter");
        }
    }

    auto thick_it = parameters.find("thickness");
    if (thick_it != parameters.end()) {
        try {
            thickness = std::stod(thick_it->second);
        } catch (...) {
            ATH_MSG_WARNING("Could not parse thickness parameter");
        }
    }

    // Create BCMPrimeDiamondDesign and store in map
    auto design = new BCMPrimeDiamondDesign(sizeX, sizeY, thickness);
    m_geometryMap[typeName] = design;

    ATH_MSG_DEBUG("Created design " << typeName
                  << ": sizeX=" << sizeX
                  << " sizeY=" << sizeY
                  << " thickness=" << thickness);
}

} // namespace InDetDD
