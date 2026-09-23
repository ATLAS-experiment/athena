/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "BCMPrimeGmxInterface.h"

#include "BCMPrimeReadoutGeometry/BCMPrimeDetectorManager.h"
#include "BCMPrimeReadoutGeometry/BCMPrimeDiamondDesign.h"
#include "InDetIdentifier/BCMPrime_ID.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "InDetSimEvent/SiHitIdHelper.h"
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "ReadoutGeometryBase/SiCommonItems.h"

#include <memory>

namespace InDetDD
{

  BCMPrimeGmxInterface::BCMPrimeGmxInterface(BCMPrimeDetectorManager* detectorManager,
                                             SiCommonItems* commonItems)
  : AthMessaging("BCMPrimeGmxInterface"),
    m_detectorManager(detectorManager),
    m_commonItems(commonItems)
{
    ATH_MSG_DEBUG("BCMPrimeGmxInterface constructed with detector manager " << m_detectorManager);
}

namespace {
int bcmPrimeOfflineEndcap(int hitBarrelEndcap)
{
  return (hitBarrelEndcap == 0) ? 4 : -4;
}
} // namespace

int BCMPrimeGmxInterface::sensorId(std::map<std::string, int> &index) const
{
  // Return the Simulation HitID (nothing to do with "ATLAS Identifiers" aka "Offline Identifiers")
  // Part=3 marks BCMPrime in the P2-RUN4 SiHitId scheme (0=pixel, 1=strip, 2=HGTD/PLR, 3=BCMPrime).
  int hitIdOfModule = SiHitIdHelper::GetHelper()->buildHitId(3,
                                                             index["barrel_endcap"],
                                                             index["layer_wheel"],
                                                             index["eta_module"],
                                                             index["phi_module"],
                                                             index["side"]);

  ATH_MSG_DEBUG("Index list: " << index["barrel_endcap"] << " " << index["layer_wheel"] << " "
                               << index["eta_module"] << " " << index["phi_module"] << " " << index["side"]);
  ATH_MSG_DEBUG("hitIdOfModule = " << std::hex << hitIdOfModule << std::dec);
  ATH_MSG_DEBUG(" bec = " << SiHitIdHelper::GetHelper()->getBarrelEndcap(hitIdOfModule)
                << " lay = " << SiHitIdHelper::GetHelper()->getLayerDisk(hitIdOfModule)
                << " eta = " << SiHitIdHelper::GetHelper()->getEtaModule(hitIdOfModule)
                << " phi = " << SiHitIdHelper::GetHelper()->getPhiModule(hitIdOfModule)
                << " side = " << SiHitIdHelper::GetHelper()->getSide(hitIdOfModule));
  return hitIdOfModule;
}

void BCMPrimeGmxInterface::addSensorType(const std::string& clas,
                                        const std::string& typeName,
                                        const std::map<std::string, std::string>& parameters)
{
  ATH_MSG_DEBUG("addSensorType called for class " << clas << ", type " << typeName);
  makeBCMPrimeDiamondDesign(typeName, parameters);
}

void BCMPrimeGmxInterface::addSensor(
    const std::string& typeName,
    std::map<std::string, int> &index,
    int /*sensitiveId*/,
    GeoVFullPhysVol* fpv)
{
    ATH_MSG_DEBUG("BCMPrime addSensor called for " << typeName);
    for (const auto& [k, v] : index) {
        ATH_MSG_DEBUG("  index[" << k << "] = " << v);
    }
    // Look up the design for this sensor type
    auto it = m_geometryMap.find(typeName);
    if (it == m_geometryMap.end()) {
        ATH_MSG_WARNING("No design found for sensor type " << typeName);
        return;
    }

    const InDetDD::SiDetectorDesign* design = it->second;
    ATH_MSG_DEBUG("BCMPrime using design " << design << " for sensor type " << typeName);
    ATH_MSG_DEBUG("BCMPrime sensor instance: type=" << typeName
                  << " barrel_endcap=" << index["barrel_endcap"]
                  << " layer_wheel=" << index["layer_wheel"]
                  << " eta_module=" << index["eta_module"]
                  << " phi_module=" << index["phi_module"]
                  << " side=" << index["side"]);

    const BCMPrime_ID* idHelper = dynamic_cast<const BCMPrime_ID*>(m_commonItems->getIdHelper());
    if (!idHelper) {
        ATH_MSG_ERROR("Failed dynamic_cast to BCMPrime_ID in BCMPrimeGmxInterface::addSensor");
        return;
    }

    const int barrelEndcap = bcmPrimeOfflineEndcap(index["barrel_endcap"]);
    const int layerWheel = index["layer_wheel"];
    const int phiModule = index["phi_module"];
    const int etaModule = index["eta_module"];
    Identifier id = idHelper->wafer_id(barrelEndcap, layerWheel, phiModule, etaModule);
    IdentifierHash hashId = idHelper->wafer_hash(id);
    if (!hashId.is_valid()) {
        ATH_MSG_ERROR("Invalid BCMPrime id for sensitive module " << typeName
                      << " bec=" << barrelEndcap
                      << " layer=" << layerWheel
                      << " phi=" << phiModule
                      << " eta=" << etaModule);
        return;
    }

    m_detectorManager->addDetectorElement(new SiDetectorElement(id, design, fpv, m_commonItems));
}

void BCMPrimeGmxInterface::makeBCMPrimeDiamondDesign(const std::string& typeName,
                                                      const std::map<std::string, std::string>& parameters)
{
    ATH_MSG_DEBUG("makeBCMPrimeDiamondDesign for type " << typeName);

    
    if (m_geometryMap.find(typeName) != m_geometryMap.end()) {
        ATH_MSG_DEBUG("Design " << typeName << " already exists");
        return;
    }


    double sizeX = 10.0;        
    double sizeY = 10.0;        
    double thickness = 0.5;     

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

    auto design = std::make_unique<BCMPrimeDiamondDesign>(sizeX, sizeY, thickness);
    const SiDetectorDesign* designPtr = m_detectorManager->addDesign(std::move(design));
    m_geometryMap[typeName] = designPtr;

    ATH_MSG_DEBUG("Created design " << typeName
                  << ": sizeX=" << sizeX
                  << " sizeY=" << sizeY
                  << " thickness=" << thickness);
}

} // namespace InDetDD
