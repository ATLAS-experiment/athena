/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonStationIndex/MuonStationIndex.h"
#include <array>

namespace Muon {
   namespace MuonStationIndex{

   ChIndex chIndex( const std::string& index) {
      if( index == "BIL" ) {
         return ChIndex::BIL;
      } else if( index == "BMS" ) {
         return ChIndex::BMS;
      } else if( index == "BIS" ) {
         return ChIndex::BIS;
      } else if( index == "BML" ) {
         return ChIndex::BML;
      } else if( index == "BOS" ) {
         return ChIndex::BOS;
      } else if( index == "BOL" ) {
         return ChIndex::BOL;
      } else if( index == "BEE" ) {
         return ChIndex::BEE;
      } else if( index == "EIS" ) {
         return ChIndex::EIS;
      } else if( index == "EIL" ) {
         return ChIndex::EIL;
      } else if( index == "EMS" ) {
         return ChIndex::EMS;
      } else if( index == "EML" ) {
         return ChIndex::EML;
      } else if( index == "EOS" ) {
         return ChIndex::EOS;
      } else if( index == "EOL" ) {
         return ChIndex::EOL;
      } else if( index == "EES" ) {
         return ChIndex::EES;
      } else if( index == "EEL" ) {
         return ChIndex::EEL;
      } else if( index == "CSS" ) {
         return ChIndex::CSS;
      } else if( index == "CSL" ) {
         return ChIndex::CSL;
      } 
      return ChIndex::ChUnknown; 
   }
   StIndex toStationIndex( DetectorRegionIndex region, LayerIndex layer ) {
      constexpr unsigned nMax = sectorLayerHashMax();
      static constexpr std::array<StIndex, nMax> regionLayerToStationIndex {
         StIndex::EI, StIndex::EM, StIndex::EO,
         StIndex::EE, StIndex::BE, StIndex::BI,
         StIndex::BM, StIndex::BO, StIndex::StUnknown,
         StIndex::StUnknown,
         StIndex::EI, StIndex::EM, StIndex::EO,
         StIndex::EE, StIndex::BE
      };
      return regionLayerToStationIndex[ sectorLayerHash( region, layer ) ];
   }

   ChIndex
   toChamberIndex( DetectorRegionIndex region, LayerIndex layer, bool isSmall ) {
      constexpr unsigned nMax = sectorLayerHashMax();
      static constexpr std::array<ChIndex, nMax> regionLayerToChamberIndexSmall {
         /** EndCapA hash */
         ChIndex::EIS, ChIndex::EMS, ChIndex::EOS, ChIndex::EES, ChIndex::BEE,
         /** Barrel hash column  BEE is not counted for the small sectors */
         ChIndex::BIS, ChIndex::BMS, ChIndex::BOS, ChIndex::ChUnknown, ChIndex::ChUnknown,
         /** EndCapC hash */
         ChIndex::EIS, ChIndex::EMS, ChIndex::EOS, ChIndex::EES, ChIndex::BEE
      };
      static constexpr std::array<ChIndex, nMax> regionLayerToChamberIndexLarge {
         /** EndCapA hash */
         ChIndex::EIL, ChIndex::EML, ChIndex::EOL, ChIndex::EEL, ChIndex::ChUnknown,
         /** Barrel hash column  BEE is not counted for the large sectors */
         ChIndex::BIL, ChIndex::BML, ChIndex::BOL, ChIndex::ChUnknown, ChIndex::ChUnknown,
         /** EndCapC hash */
         ChIndex::EIL, ChIndex::EML, ChIndex::EOL, ChIndex::EEL, ChIndex::ChUnknown,
      };
      if( isSmall ) {
         return regionLayerToChamberIndexSmall[ sectorLayerHash(region, layer)];
      }
      return regionLayerToChamberIndexLarge[sectorLayerHash(region, layer )];
   }

   const std::string& phiName( PhiIndex index ) { 
      static const std::array<std::string, toInt(PhiIndex::PhiIndexMax)> phiIndexNames = {
         "BI1", "BI2", "BM1", "BM2", "BO1", "BO2", "T1", "T2", "T3", "T4", "CSC", "STGC1",
         "STGC2"
      };

      if(index == PhiIndex::PhiUnknown ) {
         static const std::string dummy{"PhiUnknown"};
         return dummy;
      }
      if(index >= PhiIndex::PhiIndexMax) {
         static const std::string dummy{"PhiOutOfRange"};
         return dummy;
      }
      return phiIndexNames[toInt(index)];
   }

   const std::string& stName( StIndex index ) {
      static const std::array<std::string, toInt(StIndex::StIndexMax)> stationIndexNames {
         "BI", "BM", "BO", "BE", "EI", "EM", "EO", "EE"};

      if( index == StIndex::StUnknown ) {
         static const std::string dummy{"StUnknown"};
         return dummy;
      }
      if( index >= StIndex::StIndexMax) {
         static const std::string dummy{"StIndexMax"};
         return dummy;
      }
      return stationIndexNames[ toInt(index) ];
   }

   const std::string& chName( ChIndex index ) {
      static const std::array<std::string, toInt(ChIndex::ChIndexMax)> chamberIndexNames {
         "BIS", "BIL", "BMS", "BML", "BOS", "BOL", "BEE",
         "EIS", "EIL", "EMS", "EML", "EOS", "EOL", "EES",
         "EEL", "CSS", "CSL"
      };

      if( index == ChIndex::ChUnknown ) {
         static const std::string dummy{"ChUnknown"};
         return dummy;
      }
      if( index >= ChIndex::ChIndexMax ) {
         static const std::string dummy{"ChOutOfRange"};
         return dummy;
      }
      return chamberIndexNames[toInt(index)];
   }

   const std::string&
   regionName( DetectorRegionIndex index ) {
      static const std::array<std::string, toInt(DetectorRegionIndex::DetectorRegionIndexMax)> detectorRegionIndexNames {
         "EndcapA", "Barrel", "EndcapC"};

      if( index == DetectorRegionIndex::DetectorRegionUnknown ) {
         static const std::string dummy( "DetectorRegionUnknown" );
         return dummy;
      }
      if( index >= DetectorRegionIndex::DetectorRegionIndexMax ) {
         static const std::string dummy( "DetectorRegionIndexMax" );
         return dummy;
      }
      return detectorRegionIndexNames[ toInt(index) ];
   }

   const std::string& layerName( LayerIndex index ) {
      static const std::array<std::string, toInt(LayerIndex::LayerIndexMax)> layerIndexNames {
         "Inner", "Middle", "Outer", "Extended", "BarrelExtended"};

      if( index == LayerIndex::LayerUnknown ) {
         static const std::string dummy{"LayerUnknown"};
         return dummy;
      }
      if( index >= LayerIndex::LayerIndexMax) {
         static const std::string dummy{"LayerOutOfRange"};
         return dummy;
      }
      return layerIndexNames[toInt(index)];
   }

   const std::string&
   technologyName( TechnologyIndex index ) {
      using TechIdx = TechnologyIndex;
      static const std::array<std::string, toInt(TechIdx::TechnologyIndexMax)> technologyIndexNames {
         "MDT", "CSC", "RPC", "TGC", "STGC", "MM"};

      if( index == TechIdx::TechnologyUnknown ) {
         static const std::string dummy{"TechnologyUnknown"};
         return dummy;
      }
      if( index >= TechIdx::TechnologyIndexMax ) {
         static const std::string dummy{"TechnologyIndexMax"};
         return dummy;
      }
      return technologyIndexNames[ toInt(index) ];
   }


   }
} // namespace Muon
