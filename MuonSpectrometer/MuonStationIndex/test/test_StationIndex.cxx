/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonStationIndex/MuonStationIndex.h"

#include <cstdlib>
#include <iostream>
#include <set>
#include <string>

using namespace Muon::MuonStationIndex;

#define PRINT_ERROR(MSG) \
    std::cerr<<__func__<<"() "<<__LINE__<<" - "<<MSG<<std::endl; 

#define PRINT_INFO(MSG) \
    std::cout<<__func__<<"() "<<__LINE__<<" - "<<MSG<<std::endl; 


bool checkStIdxToChIdx(StIndex st, bool large) {
    
  ChIndex ch = toChamberIndex(st, !large);
  switch (st) {
      case StIndex::BI:{
        if ( (large && ch == ChIndex::BIL) || (!large && ch == ChIndex::BIS)) {
            return true;
        }
        break;
      } case StIndex::BM: {
        if ( (large && ch == ChIndex::BML) || (!large && ch == ChIndex::BMS)) {
            return true;
        }
        break;
      } case StIndex::BO: {
        if ( (large && ch == ChIndex::BOL) || (!large && ch == ChIndex::BOS)) {
            return true;
        }
        break;
      } case StIndex::BE: {
        if ( (!large && ch == ChIndex::ChUnknown) || (large && ch == ChIndex::BEE)) {
            return true;
        }
        break;
      } case StIndex::EI: {
        if ( (large && ch == ChIndex::EIL) || (!large && ch == ChIndex::EIS)) {
          return true;
        }
        break;
      } case StIndex::EM: {
        if ( (large && ch == ChIndex::EML) || (!large && ch == ChIndex::EMS)) {
          return true;
        }
        break;
      } case StIndex::EO: {
        if ( (large && ch == ChIndex::EOL) || (!large && ch == ChIndex::EOS)) {
          return true;
        }
        break;
      } case StIndex::EE: {
        if ( (large && ch == ChIndex::EEL) || (!large && ch == ChIndex::EES)) {
          return true;
        }
        break;
      } case StIndex::StIndexMax:
        case StIndex::StUnknown:
          return true;

  }
  PRINT_ERROR("Transformation of "<<stName(st)<<", large: "<<(large ? "si" : "no")<<" failed. Got "<<chName(ch)<<".");
  return false;
}

bool checkRegionIdxToChIdx(DetectorRegionIndex detIdx,
                           LayerIndex layerIdx,
                           bool large) {
    ChIndex ch = toChamberIndex(detIdx, layerIdx, !large);
    switch (detIdx) {
        case DetectorRegionIndex::DetectorRegionUnknown:
        case DetectorRegionIndex::DetectorRegionIndexMax:
            break;
        case DetectorRegionIndex::Barrel:{
            switch(layerIdx) {
                case LayerIndex::Inner:
                    return (large && ChIndex::BIL == ch) || (!large && ChIndex::BIS == ch);             
                case LayerIndex::Middle:
                   return (large && ChIndex::BML == ch) || (!large && ChIndex::BMS == ch);
                case LayerIndex::Outer:
                    return (large && ChIndex::BOL == ch) || (!large && ChIndex::BOS == ch);
                case LayerIndex::BarrelExtended:
                case LayerIndex::LayerIndexMax:
                case LayerIndex::LayerUnknown:
                case LayerIndex::Extended:
                    return ChIndex::ChUnknown == ch;
            }
            break;
        }
        case DetectorRegionIndex::EndcapA:
        case DetectorRegionIndex::EndcapC:{
            switch(layerIdx) {
                case LayerIndex::Inner:
                    return (large && ChIndex::EIL == ch) || (!large && ChIndex::EIS == ch);
                case LayerIndex::Middle:
                    return (large && ChIndex::EML == ch) || (!large && ChIndex::EMS == ch);
                case LayerIndex::Outer:
                    return (large && ChIndex::EOL == ch) || (!large && ChIndex::EOS == ch);
                case LayerIndex::Extended:
                    return (large && ChIndex::EEL == ch) || (!large && ChIndex::EES == ch);           
                case LayerIndex::BarrelExtended:
                    return (large && ChIndex::ChUnknown == ch) || (!large && ChIndex::BEE == ch);
                case LayerIndex::LayerIndexMax:
                case LayerIndex::LayerUnknown:                
                    return ChIndex::ChUnknown == ch;
            }
        }
   }   
   return false;
}

int main (){
    int exit_code = EXIT_SUCCESS;
    /// Test the assignment of the chamber index -> isSmall
    std::set<std::string> seenNames{};
    for (int ch = toInt(ChIndex::ChUnknown) + 1; ch < toInt(ChIndex::ChIndexMax); ++ch){
        const auto chIdx = static_cast<ChIndex>(ch);
        const auto name = chName(chIdx);
        PRINT_INFO("Test chamber index: "<<toInt(chIdx)<<" ("<<name<<").");
        if (!seenNames.insert(name).second){
            PRINT_ERROR("Chamber name "<<name<<" is duplciate. ");
            exit_code = EXIT_FAILURE;
        }
        if ( (name[2] == 'S') != isSmall(chIdx)) {
            PRINT_ERROR("Big/small expectation of "<<name<<" is incorrect: "<<(isSmall(chIdx) ? "si" : "no"));
            exit_code = EXIT_FAILURE;
        }
        if ( (name[0] == 'B') != isBarrel(chIdx)){
            PRINT_ERROR("Barrel/Endcap expectation of "<<name<<" is incorrect: "<<(isBarrel(chIdx) ? "si" : "no"));
            exit_code = EXIT_FAILURE;  
        }
        if (chIndex(name) != chIdx) {
            PRINT_ERROR("Backward <-> forward mapping of chIndex and name is wrong: "<<chName(chIndex(name))<<" expected: "<<name);
            exit_code = EXIT_FAILURE;
        }
        auto stIdx = toStationIndex(chIdx);
        /// Don't perform the back conversion test for CSS & CSL as they're mapped to EI
        if (chIdx == ChIndex::CSS || chIdx == ChIndex::CSL) {
            continue;
        }
        auto backChFromSt = toChamberIndex(toStationIndex(chIdx), isSmall(chIdx));
        if (backChFromSt != chIdx) {
            PRINT_ERROR("Backward <-> forward mapping of chIndex -> stIndex: "<<stName(stIdx)<<"("
                        <<toInt(stIdx)<<"),  small: "<<(isSmall(chIdx) ? "si" : "no")
                        <<" -> chIndex: "<<chName(backChFromSt));
            exit_code = EXIT_FAILURE;
        }
    }
    seenNames.clear();
    /// Test translation of the StationIndex -> chamber index using the is large flag
    for (StIndex st : {StIndex::BI, StIndex::BM, StIndex::BO, StIndex::BE, 
                     StIndex::EI, StIndex::EM, StIndex::EO, StIndex::EE}) {
        const auto name = stName(st);
        PRINT_INFO("Test station index: "<<toInt(st)<<" ("<<name<<").");
        if (name.size() != 2 || !seenNames.insert(name).second) {
            PRINT_ERROR("Station name "<<name<<" does not have 2 characters or is already inserted");
            exit_code = EXIT_FAILURE;
        }
        for (bool large : { false, true}) {
            if (!checkStIdxToChIdx(st, large)) {
                exit_code = EXIT_FAILURE;
            }
        }
    }
    for (DetectorRegionIndex detReg : {DetectorRegionIndex::EndcapA, DetectorRegionIndex::Barrel, DetectorRegionIndex::EndcapC}) {
        for (LayerIndex layer : {LayerIndex::Inner, LayerIndex::Middle, LayerIndex::Outer,
                                 LayerIndex::Extended, LayerIndex::BarrelExtended}){

            const unsigned layHash = sectorLayerHash(detReg, layer);
            PRINT_INFO("Test combination of "<<regionName(detReg)<<" ("<<toInt(detReg)<<") & "
                        <<layerName(layer)<<"("<<toInt(layer)<<") -> hash: "<<layHash);
            if (layHash >= sectorLayerHashMax()){
                PRINT_ERROR("Hash exceeds maximum: "<<sectorLayerHashMax());
                exit_code = EXIT_FAILURE;
            }
            /// Back conversion
            auto [detBack, layerBack] = decomposeSectorLayerHash(layHash);
            if ( (detBack != detReg) || (layerBack != layer)) {
                PRINT_ERROR("Back conversion resulted in "<<regionName(detBack)<<" ("<<toInt(detBack)<<") & "
                            <<layerName(layerBack)<<"("<<toInt(layerBack)<<") -> hash: "<<sectorLayerHash(detBack, layerBack));
                exit_code = EXIT_FAILURE;
            }
            for (bool large: {false, true}) {
                if (!checkRegionIdxToChIdx(detReg, layer,large)) {
                    PRINT_ERROR("Translation of large: "<<(large ? "si" : "no")
                                <<" got assigned to: "<<chName(toChamberIndex(detReg, layer, !large)));
                    exit_code = EXIT_FAILURE;
                }
           } 
       }
    }


  return exit_code;
}