/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONSTATIONINDEX_MUONSTATIONINDEX_H
#define MUONSTATIONINDEX_MUONSTATIONINDEX_H

#include <string>
#include <vector>
#include <stdint.h>

namespace Muon {
  namespace MuonStationIndex {
    /** enum to classify the different chamber layers in the muon spectrometer */
    enum class ChIndex: int8_t {
      ChUnknown = -1,
      BIS, BIL, BMS, BML, BOS, BOL, BEE,
      EIS, EIL, EMS, EML, EOS, EOL, EES, EEL, CSS, CSL,
      ChIndexMax
    };

    /** enum to classify the different station layers in the muon spectrometer */
    enum class StIndex: int8_t {
      StUnknown = -1,
      BI, BM, BO, BE,
      EI, EM, EO, EE,
      StIndexMax
    };

    /** enum to classify the different phi layers in the muon spectrometer */
    enum class PhiIndex: int8_t {
      PhiUnknown = -1,
      BI1, BI2, BM1, BM2, BO1, BO2, T1, T2, T3, T4, CSC, STGC1, STGC2,
      PhiIndexMax
    };

    /** enum to classify the different layers in the muon spectrometer */
    enum class LayerIndex: int8_t {
      LayerUnknown = -1,
      Inner, Middle, Outer, 
      Extended,       /// EE
      BarrelExtended, /// BEE 
      LayerIndexMax
    };
    
    /** enum to classify the different layers in the muon spectrometer */
    enum class DetectorRegionIndex: int8_t {
      DetectorRegionUnknown = -1,
      EndcapA, Barrel, EndcapC,
      DetectorRegionIndexMax 
    };    

    /** enum to classify the different layers in the muon spectrometer */
    enum class TechnologyIndex: int8_t {
      TechnologyUnknown = -1,
      MDT, CSC, RPC, TGC, STGC, MM,
      TechnologyIndexMax  
    };    
    /*** Convert the strong enum to an integer */
    template <typename EnumType>
    constexpr int toInt(const EnumType enumVal) {
      return static_cast<int>(enumVal);
    }
    /** convert ChIndex into StIndex */
    StIndex toStationIndex( ChIndex index );

    /** convert ChIndex into LayerIndex */
    LayerIndex toLayerIndex( ChIndex index );

    /** convert StIndex into LayerIndex */
    LayerIndex toLayerIndex( StIndex index );

    /** convert DetectorRegionIndex + LayerIndex into StIndex */
    StIndex toStationIndex( DetectorRegionIndex region, LayerIndex layer );

    /** convert DetectorRegionIndex + LayerIndex + isSmall into ChIndex */
    ChIndex toChamberIndex( DetectorRegionIndex region, LayerIndex layer, bool isSmall ) ;

    /** @brief Returns true if the chamber index points to a barrel chamber */
    bool isBarrel(const ChIndex index);
    /** @brief Returns true if the chamber index is in a small sector */
    bool isSmall(const ChIndex index);
    /** convert StIndex + isSmall into ChIndex */
    ChIndex toChamberIndex( StIndex stIndex, bool isSmall ) ;

    /** convert PhiIndex into a string */
    const std::string& phiName( PhiIndex index ) ;

    /** convert StIndex into a string */
    const std::string& stName( StIndex index ) ;

    /** convert ChIndex into a string */
    const std::string& chName( ChIndex index ) ;

    /** convert DetectorRegionIndex into a string */
    const std::string& regionName( DetectorRegionIndex index ) ;

    /** convert LayerIndex into a string */
    const std::string& layerName( LayerIndex index ) ;

    /** convert LayerIndex into a string */
    const std::string& technologyName( TechnologyIndex index ) ;

    /** create a hash out of region and layer */
    unsigned int sectorLayerHash( DetectorRegionIndex detectorRegionIndex, LayerIndex layerIndex );

    /** maximum create a hash out of region and layer */
    constexpr unsigned int sectorLayerHashMax() {
       return toInt(DetectorRegionIndex::DetectorRegionIndexMax)*toInt(LayerIndex::LayerIndexMax);
    }

    /** decompose the hash into Region and Layer */
    std::pair<DetectorRegionIndex,LayerIndex> decomposeSectorLayerHash( unsigned int hash );

    /** return total number of sectors */
    constexpr unsigned numberOfSectors() { return 16; }
    
    /** convert ChIndex name string to enum */
    ChIndex chIndex( const std::string& index );

  }
}
#include "MuonStationIndex/MuonStationIndex.icc"
#endif
