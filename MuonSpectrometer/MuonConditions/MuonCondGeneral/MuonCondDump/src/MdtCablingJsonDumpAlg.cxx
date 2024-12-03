/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "MdtCablingJsonDumpAlg.h"
#include "StoreGate/ReadCondHandle.h"
#include "MuonReadoutGeometry/MdtReadoutElement.h"
#include "CxxUtils/ArrayHelper.h"
#include "MuonCablingData/MdtMezzanineCard.h"
#include <fstream>

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

MdtCablingJsonDumpAlg::MdtCablingJsonDumpAlg(const std::string& name, ISvcLocator* pSvcLocator):
    AthAlgorithm(name,pSvcLocator) {}


StatusCode MdtCablingJsonDumpAlg::initialize(){
  ATH_CHECK(m_idHelperSvc.retrieve());
  ATH_CHECK(m_DetectorManagerKey.initialize());
  ATH_CHECK(m_cablingKey.initialize());
  return StatusCode::SUCCESS;
} 

StatusCode MdtCablingJsonDumpAlg::execute() {
  const EventContext& ctx = Gaudi::Hive::currentContext();
  ATH_MSG_INFO("Dump cabling & mezzanines into JSON file");

  SG::ReadCondHandle<MuonGM::MuonDetectorManager> detectorMgr{m_DetectorManagerKey, ctx};
  if (!detectorMgr.isValid()){
      ATH_MSG_FATAL("Failed to retrieve the Detector manager "<<m_DetectorManagerKey.fullKey());
      return StatusCode::FAILURE;
  }

  SG::ReadCondHandle<MuonMDT_CablingMap> cabling{m_cablingKey,ctx};
  if (!cabling.isValid()) {
     ATH_MSG_ERROR("Failed to retrieve the Mdt cabling "<<m_cablingKey.fullKey());
     return StatusCode::FAILURE;
  }
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
    ATH_MSG_DEBUG("Check station "<<m_idHelperSvc->toString(station_id));   
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
                ATH_MSG_ERROR("Could no retrieve a valid online channel for "<<m_idHelperSvc->toString(tube_id));
                return StatusCode::FAILURE;
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
