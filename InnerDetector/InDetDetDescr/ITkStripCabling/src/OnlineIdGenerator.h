/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef OnlineIdGenerator_h
#define OnlineIdGenerator_h

/**   
 *   @file OnlineIdGenerator.cxx
 *
 *   @brief Algorithmically generates a dummy OnlineId from an Identifier
 *
 *   @author Shaun Roe
 *   @date October 2024
**/
#include <cstdint> //uint32_t

 
class ITkStripOnlineId;
class Identifier;
class SCT_ID;


 
namespace ITkStripCabling{
  class OnlineIdGenerator{
  public:
    enum RodId : std::uint32_t {
      BARREL_A = 0x210000,
      BARREL_C = 0x220000, 
      ENDCAP_A = 0x230000, 
      ENDCAP_C = 0x240000,
      UNKNOWN = 0xFF0000
    };
    static constexpr std::uint32_t INVALID_LINK{0x0000FFFF};
    OnlineIdGenerator(SCT_ID * pITkIdHelper); 
    ITkStripOnlineId operator()(const Identifier & offId);
    //get rod by Identifier
    std::uint32_t rod(const Identifier & offId) const;
    //get rod by bec, layer_disk, eta
    std::uint32_t rod32(int bec, int layer_disk, int phi, int eta=0) const;
    //get link as lower 16 bits of 32 bit word
    std::uint32_t barrelLink16(int eta) const;
    std::uint32_t endcapLink16(int phi) const;
  
  private:
    SCT_ID * m_pITkId{};
   
  };
}
 
 
 
 
 #endif
