/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "BCMPrimeGmxInterface.h"

#include "BCMPrimeReadoutGeometry/BCMPrimeDetectorManager.h"
#include "InDetSimEvent/SiHitIdHelper.h"

namespace InDetDD
{

  BCMPrimeGmxInterface::BCMPrimeGmxInterface()
  : AthMessaging("BCMPrimeGmxInterface")
{}

int BCMPrimeGmxInterface::sensorId(std::map<std::string, int> &index) const
{
  // Return the Simulation HitID (nothing to do with "ATLAS Identifiers" aka "Offline Identifiers")
  int hitIdOfModule = SiHitIdHelper::GetHelper()->buildHitId(0, 0, index["diamond_number"], index["module_number"], 0, 0);

  ATH_MSG_DEBUG("Index list: " << index["diamond_number"] << " " << index["module_number"]);
  ATH_MSG_DEBUG("hitIdOfModule = " << std::hex << hitIdOfModule << std::dec);
  ATH_MSG_DEBUG(" dia = " << SiHitIdHelper::GetHelper()->getLayerDisk(hitIdOfModule) <<
                " mod = " << SiHitIdHelper::GetHelper()->getEtaModule(hitIdOfModule));
  return hitIdOfModule;
}


} // namespace InDetDD
