/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Class to create unique 32-bit identifiers for the RPC strips and to convert them back to the original offline variables.
// The identifiers are created by packing the following variables into a single 32-bit integer:
// NOTICE: Most of the offline variables are 1-based, but this tool assumes 0-based indexing for the identifier to be packed.
//         sector, doubletPhi, doubletR, RPCStation, stationEta, doubletZ, strip are all 1-based in the offline variables.
//    side: 1 bit (0 for negative eta, 1 for positive eta, bit 21) 
//    sector: 4 bits (0-15, corresponding to sectors 1-16, bits 17-20)
//    doubletPhi: 1 bit (0-1, corresponding to doubletPhi=1,2, bit 16)
//    doubletR: 1 bit (0-1, corresponding to doubletR=1,2, only used to distinguish BM1 and BM2, BO-BOR/BOM, no corresponding bit)
//    station: 2 bits (0-3, corresponding to stations BI/BORBOM, BM1, BM2, BO, bits 14-15)
//    chamber: 4 bits (0-15, corresponding to chambers 1-16, bits 10-13)
//    doubletZ: 2 bits (0-2, corresponding to doubletZ=1,2, bits 8-9)
//    strip: 7 bits (0-127, corresponding to strips 1-128, bits 1-7)
//    measuresPhi: 1 bit (0 for measuresEta, 1 for measuresPhi, bit 0)

#ifndef RPCSLStripConverter_H
#define RPCSLStripConverter_H 

#include "AthenaKernel/getMessageSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "MuonStationIndex/MuonStationIndex.h"

//standalone converter for SL strip identifiers
namespace L1Muon {

enum class SLRPCStation { 
   BI_BOR_BOM = 0, 
   BM1 = 1, 
   BM2 = 2, 
   BO = 3
};

class RPCSLStripConverter {
 public:
  RPCSLStripConverter() = delete;
  ~RPCSLStripConverter() = delete;

   static uint32_t pack( bool side, uint8_t sector, bool doubletPhi, 
                                       Muon::MuonStationIndex::StIndex station, uint8_t doubletR, uint8_t chamber,
                                       uint8_t doubletZ, uint8_t strip, bool measuresPhi) {
      SLRPCStation correctedStation = static_cast<SLRPCStation>(station);
      if (station == Muon::MuonStationIndex::StIndex::BO) {correctedStation = SLRPCStation::BO;}
      else if (station == Muon::MuonStationIndex::StIndex::BM && doubletR >= 1) {correctedStation = SLRPCStation::BM2;}
      
      if (station == Muon::MuonStationIndex::StIndex::BO && doubletR == 0 && (sector+1 == 11 || sector+1 == 15)) {
         //set bit 6 of strip to 1 to indicate that this is a BOR/BOM station
         //For trigger purposes, we treat the BOR/BOM stations as BI stations,
         // but we need to distinguish them from the BI stations in the other sectors
         correctedStation = SLRPCStation::BI_BOR_BOM;
         strip |= 0x40;
      }

      // Sanity check to ensure that the input values are within the expected ranges
      if (s_runSanityChecks && !SanityCheck(sector, correctedStation, doubletR, chamber, doubletZ, strip, measuresPhi)) {
         MsgStream log(Athena::getMessageSvc(), "RPCSLStripConverter");
         log << MSG::WARNING << "Found invalid input values for packing RPC SL strip identifier: "
             << "side=" << side << ", sector=" << static_cast<int>(sector) << ", doubletPhi=" << static_cast<int>(doubletPhi)
             << ", station=" << static_cast<int>(correctedStation) << ", doubletR=" << static_cast<int>(doubletR)
             << ", chamber=" << static_cast<int>(chamber) << ", doubletZ=" << static_cast<int>(doubletZ)
             << ", strip=" << static_cast<int>(strip) << ", measuresPhi=" << static_cast<int>(measuresPhi) 
             << endmsg;
      }

      return   (static_cast<uint32_t>(side)            << s_sideShift) |
               (static_cast<uint32_t>(sector)          << s_sectorShift) |
               (static_cast<uint32_t>(doubletPhi)      << s_doubletPhiShift) |
               (static_cast<uint32_t>(correctedStation)<< s_stationShift) |
               (static_cast<uint32_t>(chamber)         << s_chamberShift) |
               (static_cast<uint32_t>(doubletZ)        << s_doubletZShift)  |
               (static_cast<uint32_t>(strip)           << s_stripShift)  |
               (static_cast<uint32_t>(measuresPhi));
   }

   static void unpack( uint32_t packed, bool& side, uint8_t& sector, uint8_t& doubletPhi, 
                                    Muon::MuonStationIndex::StIndex& station, uint8_t& chamber,
                                    uint8_t& doubletZ, uint8_t& strip, bool& measuresPhi) {
      side = getSide(packed);
      sector = getSector(packed);
      doubletPhi = getDoubletPhi(packed);
      station = getRPCStation(packed);
      chamber = getChamber(packed);
      doubletZ = getDoubletZ(packed);
      strip = getStrip(packed); // mask out the bit 6 which is used to indicate BOR/BOM stations
      measuresPhi = getMeasuresPhi(packed);
   }

   static uint8_t getSide(uint32_t packed) {
      return (packed >> s_sideShift) & s_lower1BitMask;
   }
   static uint8_t getSector(uint32_t packed) {
      return ((packed >> s_sectorShift) & s_lower4BitsMask) + 1 ; // add 1 to convert back to 1-based indexing
   }
   static uint8_t getDoubletPhi(uint32_t packed) {
      return ((packed >> s_doubletPhiShift) & s_lower1BitMask) + 1; // add 1 to convert back to 1-based indexing
   }
   static SLRPCStation getStation(uint32_t packed) {
      return static_cast<SLRPCStation>((packed >> s_stationShift) & s_lower2BitsMask);
   }
   static uint8_t getChamber(uint32_t packed) {
      return ((packed >> s_chamberShift) & s_lower4BitsMask) + 1; // add 1 to convert back to 1-based indexing
   }
   static uint8_t getDoubletZ(uint32_t packed) {
      return ((packed >> s_doubletZShift) & s_lower2BitsMask) + 1; // add 1 to convert back to 1-based indexing
   }
   static uint8_t getStrip(uint32_t packed) {
      SLRPCStation corrected_station = getStation(packed);
      uint8_t sector = getSector(packed);
      if (corrected_station == SLRPCStation::BI_BOR_BOM && (sector == 11 || sector == 15)) {
         // This is a BOR/BOM station, so we need to mask out the bit 6 of the strip number
         return ((packed >> s_stripShift) & s_lower6BitsMask) + 1; // mask out bit 6
      }
      return ((packed >> s_stripShift) & s_lower7BitsMask) + 1; // add 1 to convert back to 1-based indexing
   }
   static uint8_t getMeasuresPhi(uint32_t packed) {
      return packed & s_lower1BitMask;
   }
   static uint16_t getTriggerSector(uint32_t packed) { 
      return (packed >> s_triggerSectorShift) & s_lower6BitsMask; 
   }
   
   static Muon::MuonStationIndex::StIndex getRPCStation(uint32_t packed) {
      SLRPCStation corrected_station = getStation(packed);
      uint8_t sector = getSector(packed);
      if (corrected_station == SLRPCStation::BO) { return Muon::MuonStationIndex::StIndex::BO; }
      else if (corrected_station == SLRPCStation::BI_BOR_BOM) { 
         if (sector == 11 || sector == 15) { return Muon::MuonStationIndex::StIndex::BO; }
         else { return Muon::MuonStationIndex::StIndex::BI; }
      }
      else { return Muon::MuonStationIndex::StIndex::BM; }
   }

   static uint8_t getDoubletR(uint32_t packed) {
      SLRPCStation correctedStation = getStation(packed);
      uint8_t sector = getSector(packed);
      if (correctedStation == SLRPCStation::BO) { 
         if (sector == 11 || sector == 15) { return 2; } // BO
         return 1;
      } // BO
      else if (correctedStation == SLRPCStation::BM2) { return 2; } // BM2
      else if (correctedStation == SLRPCStation::BM1) { return 1; } // BM1
      else { return 1; } // BI/BORBOM

   }

   static constexpr uint32_t s_maskStripNumber = 0xFFFFFF01; //mask to exclude strip number bits
   static constexpr uint32_t s_maskChamberGeometry = 0xFFFFC001; //mask to exclude chamber, doubletZ and strip number bits
   static constexpr uint32_t s_maskStationGeometry = 0xFFFF0001; //mask to exclude station, chamber, doubletZ and strip number bits

   private:

   static constexpr uint32_t s_lower1BitMask = 0x00000001;
   static constexpr uint32_t s_lower2BitsMask = 0x00000003;
   static constexpr uint32_t s_lower4BitsMask = 0x0000000F;
   static constexpr uint32_t s_lower6BitsMask = 0x0000003F;
   static constexpr uint32_t s_lower7BitsMask = 0x0000007F;

   static constexpr int s_maxSector = 15; // maximum sector value (0-15 for sectors 1-16)
   static constexpr SLRPCStation s_maxStation = SLRPCStation::BO; // maximum station value (0-3 for stations BI, BM1, BM2, BO)
   static constexpr int s_maxDoubletR = 1; // maximum doubletR value (0 for doubletR=1, 1 for doubletR=2)
   static constexpr int s_maxChamber = 15; // maximum chamber value (0-15 for chambers 1-16)
   static constexpr int s_maxDoubletZ = 2; // maximum doubletZ value (0-2 for doubletZ=1,2,3)
   static constexpr int s_maxStrip = 127; // maximum strip value (0-127 for strips 1-128)

   static constexpr int s_sideShift = 21;
   static constexpr int s_sectorShift = 17;
   static constexpr int s_doubletPhiShift = 16;
   static constexpr int s_stationShift = 14;
   static constexpr int s_chamberShift = 10;
   static constexpr int s_doubletZShift = 8;
   static constexpr int s_stripShift = 1;
   static constexpr int s_triggerSectorShift = 16;

   static constexpr uint16_t s_nullBIStrip = 0x0000;
   static constexpr uint16_t s_nullBM1Strip = 0x4000;
   static constexpr uint16_t s_nullBM2Strip = 0x8000;
   static constexpr uint16_t s_nullBOStrip = 0xC000;

   static constexpr bool s_runSanityChecks = true; // Set to true to enable sanity checks on input values
   static bool SanityCheck( uint8_t sector, 
                     SLRPCStation correctedStation, uint8_t doubletR, uint8_t chamber,
                     uint8_t doubletZ, uint8_t strip, bool measuresPhi) {
      MsgStream log(Athena::getMessageSvc(), "RPCSLStripConverter");
      if (sector > s_maxSector) {
         log << MSG::WARNING << "Sanity Check Failed: Invalid sector value: " << static_cast<int>(sector) << endmsg;
         return false; }
      if (correctedStation > s_maxStation) { 
         log << MSG::WARNING << "Sanity Check Failed: Invalid station value: " << static_cast<int>(correctedStation) << endmsg;
         return false; }
      if (doubletR > s_maxDoubletR) { 
         log << MSG::WARNING << "Sanity Check Failed: Invalid doubletR value: " << static_cast<int>(doubletR) << endmsg;
         return false; }
      if (chamber > s_maxChamber) { 
         log << MSG::WARNING << "Sanity Check Failed: Invalid chamber value: " << static_cast<int>(chamber) << endmsg;
         return false; }
      if (doubletZ > s_maxDoubletZ) { 
         log << MSG::WARNING << "Sanity Check Failed: Invalid doubletZ value: " << static_cast<int>(doubletZ) << endmsg;
         return false; }
      if (strip > s_maxStrip) { 
         log << MSG::WARNING << "Sanity Check Failed: Invalid strip value: " << static_cast<int>(strip) << endmsg;
         return false; }

      if (correctedStation == SLRPCStation::BI_BOR_BOM && (sector == 11 || sector == 15)) {
         // This is a BOR/BOM station, so we need to ensure that the strip number is within the valid range
         strip &= s_lower6BitsMask; // mask out bit 6
      }

      uint32_t strip_16bit = (static_cast<uint32_t>(correctedStation) << s_stationShift) |
                           (static_cast<uint32_t>(chamber)          << s_chamberShift) |
                           (static_cast<uint32_t>(doubletZ)         << s_doubletZShift) |
                           (static_cast<uint32_t>(strip)            << s_stripShift) |
                           (static_cast<uint32_t>(measuresPhi));

      if (correctedStation == SLRPCStation::BI_BOR_BOM && (strip_16bit >= s_nullBM1Strip)) {
         log << MSG::WARNING << "Sanity Check Failed for BI: " << std::hex << strip_16bit << std::dec << endmsg;
         return false;
      }
      if (correctedStation == SLRPCStation::BM1 && (strip_16bit >= s_nullBM2Strip || strip_16bit < s_nullBM1Strip)) {
         log << MSG::WARNING << "Sanity Check Failed for BM1: " << std::hex << strip_16bit << std::dec << endmsg;
         return false;
      }
      if (correctedStation == SLRPCStation::BM2 && (strip_16bit >= s_nullBOStrip || strip_16bit < s_nullBM2Strip)) {
         log << MSG::WARNING << "Sanity Check Failed for BM2: " << std::hex << strip_16bit << std::dec << endmsg;
         return false;
      }
      if (correctedStation == SLRPCStation::BO && strip_16bit < s_nullBOStrip) {
         log << MSG::WARNING << "Sanity Check Failed for BO: " << std::hex << strip_16bit << std::dec << endmsg;
         return false;
      }
            
      return true;
   }

};

}   // end of namespace

#endif  // RPCSLStripConverter_H
