/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "MdtCablingJsonDumpAlg.h"
#include "StoreGate/ReadCondHandle.h"
#include "MuonReadoutGeometry/MdtReadoutElement.h"
#include "CxxUtils/ArrayHelper.h"
#include "MuonCablingData/MdtMezzanineCard.h"
#include "MuonVisualizationHelpersR4/VisualizationHelpers.h"
#include <fstream>
#include <format>

using namespace MuonValR4;
namespace {
   struct TdcIdentifier: public MdtCablingOnData {
        uint8_t tdcId{0};
        ///Equality operator
        bool operator==(const TdcIdentifier& other) const noexcept{
            return this->MdtCablingOnData::operator==(other) && tdcId == other.tdcId;
        }
        bool operator<(const TdcIdentifier& other) const noexcept {
          if (this->MdtCablingOnData::operator!=(other)) return this->MdtCablingOnData::operator<(other);
        return tdcId < other.tdcId;
     }
     TdcIdentifier(const MdtCablingData& cabling):
        MdtCablingOnData{cabling},
        tdcId{cabling.tdcId} {} 
   };

  using Mapping = MdtMezzanineCard::Mapping;
  using OfflineCh = MdtMezzanineCard::OfflineCh;

}



StatusCode MdtCablingJsonDumpAlg::initialize(){
  ATH_CHECK(m_idHelperSvc.retrieve());
  ATH_CHECK(m_DetectorManagerKey.initialize());
  ATH_CHECK(m_cablingKey.initialize());
  return StatusCode::SUCCESS;
} 


StatusCode MdtCablingJsonDumpAlg::execute() {
  const EventContext& ctx = Gaudi::Hive::currentContext();
  ATH_MSG_INFO("Dump cabling & mezzanines into JSON file");

  const MuonGM::MuonDetectorManager* detectorMgr{nullptr};
  ATH_CHECK(SG::get(detectorMgr, m_DetectorManagerKey, ctx));

  const MuonMDT_CablingMap* cabling{nullptr};
  ATH_CHECK(SG::get(cabling, m_cablingKey,ctx));

  const MdtIdHelper& idHelper = m_idHelperSvc->mdtIdHelper();

  std::vector<MdtMezzanineCard> cached_cards{};
  std::set<MdtCablingData> cached_chnls{};
  constexpr auto unsetArray{make_array<uint8_t,24>(MdtMezzanineCard::NOTSET)};
  for (auto det_itr = idHelper.detectorElement_begin(); det_itr != idHelper.detectorElement_end(); ++det_itr){
    const MuonGM::MdtReadoutElement* readEle = detectorMgr->getMdtReadoutElement(*det_itr);
    if (!readEle) {
        ATH_MSG_DEBUG("Detector element does not exist. ");
        continue;
    }
    const Identifier station_id = idHelper.elementID(readEle->identify());
    if(m_insertBISCabling && m_idHelperSvc->stationNameString(station_id)=="BIS" && readEle->getStationEta() < 7){
      ATH_MSG_DEBUG("Will skip cabling for station "
                  <<m_idHelperSvc->toStringChamber(readEle->identify()) 
                  << " since it will be inserted manually");
      continue;
    }
  
    /// Struct to perform the mapping between online offline tube
    MdtMezzanineCard dummy_card(Mapping{}, readEle->getNLayers(), -1);
    std::map<TdcIdentifier, Mapping> chamber_mezz{};
    const int nLayers{readEle->getNLayers()};
    const int nTubes{readEle->getNtubesperlayer()};
    const int multiLayer{readEle->getMultilayer()};

    for (int layer = 1 ; layer <= nLayers; ++layer){
      for (int tubeInLayer = 1 ; tubeInLayer <= nTubes; ++tubeInLayer) {
          bool is_valid{false};
          const Identifier tube_id = idHelper.channelID(station_id, multiLayer, 
                                                        layer, tubeInLayer, is_valid);
              if (!is_valid) {
                ATH_MSG_VERBOSE("Invalid element");
                continue;
              }
              /// Create the cabling object
              MdtCablingData cabling_data{};
              cabling->convert(tube_id,cabling_data);
              /// Test if the online channel can be found
              if (!cabling->getOnlineId(cabling_data, msgStream())) {
                    ATH_MSG_WARNING("Could no retrieve a valid online channel for "<<m_idHelperSvc->toString(tube_id));
                    continue;
              }
              const TdcIdentifier tdc_id{cabling_data};
              chamber_mezz.try_emplace(tdc_id,unsetArray);
              chamber_mezz[tdc_id][cabling_data.channelId] = dummy_card.tubeNumber(layer, tubeInLayer);
         }
    }
    /// Next step is to filter all the mezzanine cards
    for (auto&[tdc, mezz_mapping] : chamber_mezz) {
        MdtCablingData mezzCablingId{};
        
        /// Assign MROD / SubDet / CSM
        static_cast<MdtCablingOnData&>(mezzCablingId) = tdc;
        mezzCablingId.tdcId = tdc.tdcId;
        
        MdtTdcOnlSorter chipInCab =  cabling->getOnlineConvMap().at(tdc).all_modules.at(tdc.tdcId);
        /// Offline Identifier (stationIdx, eta,phi, ml)
        static_cast<MdtCablingOffData&>(mezzCablingId) = chipInCab->offId();
        /// Tube zero
        mezzCablingId.tube = chipInCab->tubeZero();
        
        const uint8_t tubeOffSet = (mezzCablingId.tube-1)%dummy_card.numTubesPerLayer();
        /// The staggering of the hedgehog cards does not coincide with the multi-layer staggering
        ///  i.e. tubeZero is 2,3,4,5 or 2,3,4,5,6,7.
        if (tubeOffSet) {
           MdtMezzanineCard remap{mezz_mapping, dummy_card.numTubeLayers(), 0};
           for (size_t chan = 0 ; chan < mezz_mapping.size(); ++chan) {           
              const OfflineCh tube_lay = remap.offlineTube(chan, msgStream());
              if (!tube_lay.isValid) continue;
              uint8_t tubeNumber = tube_lay.tube + tubeOffSet + 1;
              mezz_mapping[chan] = remap.tubeNumber(tube_lay.layer, tubeNumber);
           }
        }
        /// Check whether the same layout is already used somewhere
        Mapping& mtmp = mezz_mapping; // Work around clang15 compilation error.
        std::vector<MdtMezzanineCard>::const_iterator itr = std::find_if(cached_cards.begin(), cached_cards.end(), 
            [&dummy_card, &mtmp](const MdtMezzanineCard& card ){
               if (dummy_card.numTubeLayers() != card.numTubeLayers()) return false;
               for (size_t ch =0; ch < mtmp.size(); ++ch) {
                  if (mtmp[ch] != card.tdcToTubeMap()[ch]) return false;
               }
              return true;
        });
        if (itr != cached_cards.end()) {
          mezzCablingId.mezzanine_type = itr->id();
        } else {
          cached_cards.emplace_back(mezz_mapping, dummy_card.numTubeLayers(), cached_cards.size() + 10);
          if (!cached_cards.back().checkConsistency(msgStream())) {
            ATH_MSG_ERROR("Wrong assignment for "<<mezzCablingId);
            return StatusCode::FAILURE;
          }
          mezzCablingId.mezzanine_type = cached_cards.back().id();
        }
        cached_chnls.insert(std::move(mezzCablingId));
    }
  }

  if(m_insertBISCabling){
    ATH_MSG_INFO("Inserting BIS cabling manually");
    // defining the costum mapping to be inserted for BIS chambers
    
    /* The BIS chambers are build using two different tube stagering configurations, one for the A side and one for the C side
    For sectors 12 and 16 the chambers of the A and C side are swapped following space constrains in the detector,
    bringing the readout side of those chambers to the other side. This also swaps the tube staggering configuration.
    Following the staggering configuration two types of mezzanine cards are used to read out the BIS chambers. 

    Mezanine type 6 are used for the A-side type chambers. It has the following channel mapping:
    
    23  19  15  11  7   3
      22  18  14  10  6   2
    21  17  13  9   5   1 
      20  16  12  8   4   0
    
    For the C-side type chambers mezzanine type 7 is used, with the following channel mapping:

      1   5   9   13  17  21
    0   4   8   12  16  20
      3   7   11  15  19  23
    2   6   10  14  18  22

    For the mezz type 7 channel 0 is connected to layer 3  while for mezz type 6 channel 0 is connected to layer 4 so below the mapping will be rotated accordingly. Since the chambers have Nx6 - 2 tubes there are special mezzanine cards with a 4x5 configuration. Since they maintain the above design but missing the last row of tubes (the row on the opposite side of channel 0) so a 6b and 7b design will be introduced with 4 channels set to read tube 255 as proxy for not connected channels. 
    
    The CSM modules of each chamber can read up to 20 mezzanine cards. For BIS 2-7 this is enough to read out all the tube of the chamber.
    For the BIS1 chambers the 24 mezzanine cards are readout by two CSMs each reading out 12 mezzanine cards. In general the channel number of the CSM to which the mezzanine is connected to is even for mezzanine cards connected to multilayer 1 and odd for multilayer 2, starting to count from 0. The channel number is increased in the direction of the global z coordinate axis. 
    */
    constexpr std::array<uint8_t,24> mezzanineType6InvertedMap{23, 17, 11, 5,
                                                               22, 16, 10, 4,
                                                                21, 15, 9, 3,
                                                                20, 14, 8, 2,
                                                                19, 13, 7, 1,
                                                                18, 12, 6, 0};
    
    constexpr std::array<uint8_t,24> mezzanineType6bInvertedMap{250, 250, 250,250,
                                                                22, 16, 10, 4,
                                                                21, 15, 9, 3,
                                                                20, 14, 8, 2,
                                                                19, 13, 7, 1,
                                                                18, 12, 6, 0};
    constexpr std::array<uint8_t,24> mezzanineType6b2InvertedMap{250, 250, 250, 250,
                                                                 23, 17, 11, 5,
                                                                 22, 16, 10, 4,
                                                                 21, 15, 9, 3,
                                                                 20, 14, 8, 2,
                                                                 19, 13, 7, 1};
    
    constexpr std::array<uint8_t,24> mezzanineType7Map{12, 18, 0, 6,
                                                       13, 19, 1, 7,
                                                       14, 20, 2, 8,
                                                       15, 21, 3, 9,
                                                       16, 22, 4, 10,
                                                       17, 23, 5, 11};
    
    constexpr std::array<uint8_t,24> mezzanineType7bMap{12, 18, 0, 6,
                                                       13, 19, 1, 7,
                                                       14, 20, 2, 8,
                                                       15, 21, 3, 9,
                                                       16, 22, 4, 10,
                                                       250, 250, 250, 250};
    
    constexpr std::array<uint8_t,24> mezzanineType7b2Map{ 13, 19, 1, 7,
                                                          14, 20, 2, 8,
                                                          15, 21, 3, 9,
                                                          16, 22, 4, 10,
                                                          17, 23, 5, 11,
                                                          250, 250, 250, 250};


  
    // Let's add the four types to the cached cards if not already present so we can reference them later
    std::map<std::string, MdtMezzanineCard> mezzTypes{};
    mezzTypes.emplace(std::make_pair("mezzanineType6", MdtMezzanineCard(mezzanineType6InvertedMap, 4, 0)));
    mezzTypes.emplace(std::make_pair("mezzanineType6b", MdtMezzanineCard(mezzanineType6bInvertedMap, 4, 0)));
    mezzTypes.emplace(std::make_pair("mezzanineType6b2", MdtMezzanineCard(mezzanineType6b2InvertedMap, 4, 0)));
    mezzTypes.emplace(std::make_pair("mezzanineType7", MdtMezzanineCard(mezzanineType7Map, 4, 0)));
    mezzTypes.emplace(std::make_pair("mezzanineType7b", MdtMezzanineCard(mezzanineType7bMap, 4, 0)));
    mezzTypes.emplace(std::make_pair("mezzanineType7b2", MdtMezzanineCard(mezzanineType7b2Map, 4, 0)));
    std::map<std::string, int> mezzTypeIds{};
    for(auto& [type,card] : mezzTypes){
      auto itr = std::ranges::find_if(cached_cards,
        [&card](const MdtMezzanineCard& cachedCard){
          if(cachedCard.numTubeLayers() != card.numTubeLayers()) {
              return false;
          }
          for(size_t ch = 0; ch < card.tdcToTubeMap().size(); ++ch){
            if(cachedCard.tdcToTubeMap()[ch] != card.tdcToTubeMap()[ch]) {
                return false;
            }
          }
          return true;
        });
      if(itr != cached_cards.end()){
        mezzTypeIds[type] = itr->id();
      } else {
        cached_cards.emplace_back(card.tdcToTubeMap(), card.numTubeLayers(), cached_cards.size() + 10);
        mezzTypeIds[type] = cached_cards.back().id();
      } 
    }
    
    for(auto& [type,card] : mezzTypes){
      ATH_MSG_VERBOSE("mezzz type" << type << " has id " << mezzTypeIds[type]);
    }
    /* We have to keep track of the MROD and CSM numbers for each partition.
    Since there are no MRODs in phase two we can assign the new BIS stations to fantasy MROD numbers while we wait for the new FELIX based cabling scheme. Since there are up to 51 MRODs per partition in RUN3 the new chambers will be assigned to MRDOs starting from 55 upwards. Each MROD can read up to 6 CSMs so the CSM counter will be increased accordingly when we assign the cabling for each chamber.
    */
    std::map<uint8_t, uint8_t> mrodMap, csmMap;
    mrodMap[97] = 55; csmMap[97] = 0; // A-side barrel
    mrodMap[98] = 55; csmMap[98] = 0; // C-side barrel

    for (auto det_itr = idHelper.detectorElement_begin(); det_itr != idHelper.detectorElement_end(); ++det_itr){
      const MuonGM::MdtReadoutElement* readEle = detectorMgr->getMdtReadoutElement(*det_itr);
      if (!readEle) {
          ATH_MSG_DEBUG("Detector element does not exist. ");
          continue;
      }
      // since BIS 1-6 chambers have identical layouts for multilayer 1 and 2 and for BIS78C the second multilayer has more tubes, so we will loop only over the multilayer 2 readout elements here and take care of the multilayer 1 cabling at the same time 
      if(readEle->getMultilayer()!=2)  {
        continue;
      }
      const Identifier station_id = idHelper.elementID(readEle->identify());
      if(  !m_insertBISCabling || m_idHelperSvc->stationNameString(station_id)!="BIS" || readEle->getStationEta() >= 7) {
        continue;
      }
      //find the multilayer 1 readout element to get the number of tubes per layer
      Identifier id_ml1 = idHelper.multilayerID(readEle->identify(),1);
      const MuonGM::MdtReadoutElement* readEle_ml1 = detectorMgr->getMdtReadoutElement(id_ml1);
      int tubesPerLayer_ml1 = readEle_ml1->getNtubesperlayer(); 

      ATH_MSG_INFO("Will insert cabling for station "<<m_idHelperSvc->toString(station_id));
      
      const int nTubes{readEle->getNtubesperlayer()};
      uint8_t tdcId{0};
      /// Subdetector identifiers 97: barrel (A), 98: barrel (C), 99 endcap (A), 100, endcap (C)
      uint8_t subdetectorId{0};
      if(readEle->getStationEta() > 0){
           subdetectorId = 97; 
      } else{
            subdetectorId = 98;
      }
      for(int tubeInLayer = 1 ; tubeInLayer <= nTubes; 
              tubeInLayer +=   mezzTypes.at("mezzanineType6").numTubesPerLayer()){
        
        if(tubeInLayer==1 && std::abs(readEle->getStationEta()) !=7){tubeInLayer -=1;} // account for the fact that the first mezzanine card only has 5 tubes per layer for BIS 1-6
        MdtCablingData mezzCablingId_ml1{};
        if(tubeInLayer <= tubesPerLayer_ml1){
          mezzCablingId_ml1.stationIndex = m_idHelperSvc->stationName(station_id);
          mezzCablingId_ml1.eta = readEle->getStationEta();
          mezzCablingId_ml1.phi = readEle->getStationPhi();
          mezzCablingId_ml1.multilayer = 1;
          mezzCablingId_ml1.subdetectorId = subdetectorId;
          mezzCablingId_ml1.csm = csmMap[subdetectorId];
          mezzCablingId_ml1.mrod = mrodMap[subdetectorId];
          mezzCablingId_ml1.tdcId = tdcId;
          mezzCablingId_ml1.tube = tubeInLayer;
        }

        
        MdtCablingData mezzCablingId_ml2{};
        mezzCablingId_ml2.stationIndex = m_idHelperSvc->stationName(station_id);
        mezzCablingId_ml2.eta = readEle->getStationEta();
        mezzCablingId_ml2.phi = readEle->getStationPhi();
        mezzCablingId_ml2.multilayer = 2;
        mezzCablingId_ml2.subdetectorId = subdetectorId;
        mezzCablingId_ml2.csm = csmMap[subdetectorId];
        mezzCablingId_ml2.mrod = mrodMap[subdetectorId];
        mezzCablingId_ml2.tdcId = tdcId +1;
        mezzCablingId_ml2.tube = tubeInLayer;


        // A-side type chamber
        std::string mezzEndString = "";
        //The first and the last mezzanine cards of BIS1-6 chambers have a 4x5 configuration
        if(std::abs(readEle->getStationEta()) !=7 &&  tubeInLayer + 6 > nTubes) { mezzEndString = "b"; }
        if(std::abs(readEle->getStationEta()) !=7 &&  tubeInLayer==0  ) { mezzEndString = "b2"; }
        ATH_MSG_VERBOSE("For station "<<m_idHelperSvc->toString(station_id) << " tube " << tubeInLayer<< " assigning mezzanine types for tubeInLayer "<<tubeInLayer <<" mezzEndString "<<mezzEndString);
        if(readEle->getStationEta() > 0 ){
          // swapped chamber in sectors 12 and 16
          if(readEle->getStationPhi() == 6 || readEle->getStationPhi() == 8){
            if(tubeInLayer <= tubesPerLayer_ml1) {
                mezzCablingId_ml1.mezzanine_type = mezzTypeIds["mezzanineType7"+mezzEndString]; // type 7 is used for type C-side chambers
            }
            mezzCablingId_ml2.mezzanine_type = mezzTypeIds["mezzanineType7"+mezzEndString]; // type 7 is used for type C-side chambers
          } else {
            if(tubeInLayer <= tubesPerLayer_ml1) {
               mezzCablingId_ml1.mezzanine_type = mezzTypeIds["mezzanineType6"+mezzEndString];
            }
            mezzCablingId_ml2.mezzanine_type = mezzTypeIds["mezzanineType6"+mezzEndString];
          }
        } else {
          // swapped chamber in sectors 12 and 16
          if(readEle->getStationPhi() == 6 || readEle->getStationPhi() == 8){
            if(tubeInLayer <= tubesPerLayer_ml1){
              mezzCablingId_ml1.mezzanine_type = mezzTypeIds["mezzanineType6"+mezzEndString]; // type 6 is used for type A-side chambers
            }
            mezzCablingId_ml2.mezzanine_type = mezzTypeIds["mezzanineType6"+mezzEndString]; // type 6 is used for type A-side chambers
          } else {
            if(tubeInLayer <= tubesPerLayer_ml1){
               mezzCablingId_ml1.mezzanine_type = mezzTypeIds["mezzanineType7"+mezzEndString];
            }
            mezzCablingId_ml2.mezzanine_type = mezzTypeIds["mezzanineType7"+mezzEndString];
          }
        }
        if(tubeInLayer <= tubesPerLayer_ml1) {
          cached_chnls.insert(std::move(mezzCablingId_ml1));
        }
        cached_chnls.insert(std::move(mezzCablingId_ml2));
        // Update identifier
        tdcId+=2;
        // since BIS1 and BIS78C chambers are read out by two CSMs we need to update the tdcId and CSM numbers accordingly
        if((std::abs(readEle->getStationEta()) == 1  && tdcId >=12) ||
                   (readEle->getStationEta() == -7  && tdcId >=17) ){  
         
          tdcId = 0;
          csmMap[subdetectorId] +=1;
          if(csmMap[subdetectorId]>5){
            csmMap[subdetectorId]=0;
            mrodMap[subdetectorId]+=1;
          }
        }
      }
      // Update CSM and MROD for next chamber
      csmMap[subdetectorId] +=1;
      if(csmMap[subdetectorId]>5){
        csmMap[subdetectorId]=0;
        mrodMap[subdetectorId]+=1;
      }
    }
  }    
    /// Summary file
    {
        std::ofstream summary{m_summaryTxt};
        if (!summary.good()) {
          ATH_MSG_ERROR("Failed to write "<<m_summaryTxt);
          return StatusCode::FAILURE;
        }
        summary<<"Extracted "<<cached_cards.size()<<" mezzanine card layouts and "
              <<cached_chnls.size()<<" chamber channels. \n\n\n";
        for (const MdtMezzanineCard& card : cached_cards) {
          summary<<card;
          MdtCablingOffData chamb{};
          for (const MdtCablingData& cabling : cached_chnls) {
              if (cabling.mezzanine_type != card.id()) continue;
              if (chamb != cabling) {
                chamb = cabling;
                summary<<std::endl<<" *** "<<idHelper.stationNameString(chamb.stationIndex);             
                summary<<static_cast<int>(std::abs(chamb.eta));
                summary<<(chamb.eta > 0 ? "A" : "C");
                summary<<static_cast<int>(chamb.phi);
                summary<<"M"<<static_cast<int>(chamb.multilayer);
                summary<<" --- tdcs: ";
              }
              summary<<static_cast<int>(cabling.tdcId)<<", ";
          }
          summary<<"\n\n"
                  <<"##############################################################\n";
        }
    }
    /// Write mezzanine file
    {
      std::ofstream mezz_json{m_mezzJSON};
      if (!mezz_json.good()) {
          ATH_MSG_ERROR("Failed to write "<<m_summaryTxt);
          return StatusCode::FAILURE;
      }
      mezz_json<<"["<<std::endl;
      for (size_t i = 0; i < cached_cards.size() ; ++i) {
         const MdtMezzanineCard& card  = cached_cards[i];
         mezz_json<<"     {\n";
         mezz_json<<"       \"mezzId\": "<<static_cast<int>(card.id())<<",\n";
         mezz_json<<"       \"nTubeLayer\": "<<static_cast<int>(card.numTubeLayers())<<",\n";
         mezz_json<<"       \"tdcToTubeMap\": [";
         for (size_t ch = 0 ; ch < card.tdcToTubeMap().size(); ++ch) {
           mezz_json<<static_cast<int>(card.tdcToTubeMap()[ch]);
           if (ch + 1 != card.tdcToTubeMap().size())mezz_json<<",";
         }
         mezz_json<<"]\n";
         mezz_json<<"     }";
         if (i +1 != cached_cards.size()) mezz_json<<",";
         mezz_json<<"\n";
      }
      mezz_json<<"]";
    }
    {
      std::ofstream chamb_json{m_cablingJSON};
      if (!chamb_json.good()) {
         ATH_MSG_FATAL("Failed to write "<<m_cablingJSON);
         return StatusCode::FAILURE;
      }
      chamb_json<<"[\n";
      size_t i =0;
      for (const MdtCablingData& chamb : cached_chnls){ 
        chamb_json<<"    {\n";
        chamb_json<<"     \"station\": \""<<idHelper.stationNameString(chamb.stationIndex)<<"\",\n";
        chamb_json<<"     \"eta\": "<<static_cast<int>(chamb.eta)<<",\n";
        chamb_json<<"     \"phi\": "<<static_cast<int>(chamb.phi)<<",\n";
        chamb_json<<"     \"ml\": "<<static_cast<int>(chamb.multilayer)<<",\n";
        chamb_json<<"     \"subDet\": "<<static_cast<int>(chamb.subdetectorId)<<",\n";
        chamb_json<<"     \"csm\": "<<static_cast<int>(chamb.csm)<<",\n";
        chamb_json<<"     \"mrod\": "<<static_cast<int>(chamb.mrod)<<",\n";
        chamb_json<<"     \"tdcId\": "<<static_cast<int>(chamb.tdcId)<<",\n";
        chamb_json<<"     \"mezzId\": "<<static_cast<int>(chamb.mezzanine_type)<<",\n";
        chamb_json<<"     \"tubeZero\": "<<static_cast<int>(chamb.tube)<<"\n";
        chamb_json<<"    }";
        if (i +1 != cached_chnls.size()) chamb_json<<",";
        chamb_json<<"\n";
        ++i;
      }
      chamb_json<<"]"<<std::endl;
    }

    
  return StatusCode::SUCCESS;
} 
