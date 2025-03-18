/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "OnlineIdGenerator.h"
#include "ITkStripCabling/ITkStripOnlineId.h"
#include "Identifier/Identifier.h"
#include "InDetIdentifier/SCT_ID.h"

#include <string>
#include <cmath>

namespace ITkStripCabling{
//hardcoded: to be replaced later

  OnlineIdGenerator::OnlineIdGenerator(SCT_ID * sctId):m_pITkId(sctId){
  }

  std::uint32_t 
  OnlineIdGenerator::rod32(int bec, int layer_disk, int phi, int eta) const{
    std::uint32_t result = RodId::UNKNOWN;
    const auto absBec = std::abs(bec);
    if (bec == 0){
      result = (eta>0) ? BARREL_A : BARREL_C;
      result |= (static_cast<std::uint32_t>(phi)<<8);
    } else if (absBec == 2){
      result = (bec>0)? ENDCAP_A:ENDCAP_C;
      result |= (static_cast<std::uint32_t>(std::abs(eta))<<8);
    }
    return result+layer_disk;
  }
  
  std::uint32_t 
  OnlineIdGenerator::rod(const Identifier & offId) const{
    const int bec =  m_pITkId->barrel_ec(offId);
    const int eta = m_pITkId->eta_module(offId);
    const int layer_disk = m_pITkId->layer_disk(offId);
    const int phi = m_pITkId->phi_module(offId);
    return rod32(bec, layer_disk, phi, eta);
  }
  
  std::uint32_t 
  OnlineIdGenerator::barrelLink16(int eta) const{
    return static_cast< std::uint32_t>(std::abs(eta));
  }
  std::uint32_t 
  OnlineIdGenerator::endcapLink16(int phi) const{
    return static_cast< std::uint32_t>(phi);
  }
  
  ITkStripOnlineId 
  OnlineIdGenerator::operator()(const Identifier & offId){
    const int bec =  m_pITkId->barrel_ec(offId);
    const auto absBec = std::abs(bec);
    const int eta = m_pITkId->eta_module(offId);
    const int layer_disk = m_pITkId->layer_disk(offId);
    const int phi = m_pITkId->phi_module(offId);
    const std::uint32_t rod = rod32(bec, layer_disk, phi, eta);
    const std::uint32_t fibre = (absBec == 0)? barrelLink16(eta) : endcapLink16(phi);
    return ITkStripOnlineId(rod,fibre);
  }


}
