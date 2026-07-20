/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MdtToyCablingJsonDumpAlg.h"

#include <MuonCablingData/MuonMDT_CablingMap.h>
#include <MuonCablingData/MdtMezzanineCard.h>

#include "nlohmann/json.hpp"
#include <fstream>



MdtToyCablingJsonDumpAlg::MdtToyCablingJsonDumpAlg(const std::string& name, ISvcLocator* pSvcLocator):
    AthAlgorithm{name, pSvcLocator} {}

StatusCode MdtToyCablingJsonDumpAlg::initialize() {
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(detStore()->retrieve(m_detMgr));
    if (!m_idHelperSvc->hasMDT()) {
        ATH_MSG_FATAL("You can't write mdt cablings without mdt detectors? ");
        return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
}
StatusCode MdtToyCablingJsonDumpAlg::execute(const EventContext& /*ctx*/) {
  
  
  
  using Mapping = MdtMezzanineCard::Mapping;

  Mapping tubeMapping{};
  /// An hommage to the Oktoberfest chambers
  for (int channel = 0; channel<12; ++channel) {
      tubeMapping[2*channel] = channel;
      tubeMapping[2*channel +1] = 23 -channel;

  }
  const MdtMezzanineCard mapping4Layers{tubeMapping, 4, 1};
  const MdtMezzanineCard mapping3Layers{tubeMapping, 3, 2};
  

  {
      std::ofstream outStream{m_mezzanineJSON};
      if (!outStream.good()) {
        ATH_MSG_FATAL("Failed to create JSON file "<<m_mezzanineJSON);
        return StatusCode::FAILURE;
      }
      auto dumpCard = [&outStream](const MdtMezzanineCard& card) {
          outStream<<"     {"<<std::endl;
          outStream<<"       \"mezzId\": "<<static_cast<int>(card.id())<<","<<std::endl;
          outStream<<"       \"nTubeLayer\": "<<static_cast<int>(card.numTubeLayers())<<","<<std::endl;
          outStream<<"       \"tdcToTubeMap\": [";
          for (size_t ch = 0 ; ch < card.tdcToTubeMap().size(); ++ch) {
              outStream<<static_cast<int>(card.tdcToTubeMap()[ch]);
              if (ch + 1 != card.tdcToTubeMap().size()) {
                 outStream<<",";
              }
          }
          outStream<<"]"<<std::endl;
          outStream<<"     }";
      };
      outStream<<"["<<std::endl;
      dumpCard(mapping4Layers);
      outStream<<","<<std::endl;
      dumpCard(mapping3Layers);      
      outStream<<"]"<<std::endl;
  }

  ATH_MSG_VERBOSE("Test mappings "<<std::endl<<mapping4Layers<<std::endl<<mapping3Layers);
  
  std::vector<const MuonGMR4::MdtReadoutElement*> reEles{m_detMgr->getAllMdtReadoutElements()};

  /// Subdetector identifiers 97: barrel (A), 98: barrel (C), 99 endcap (A), 100, endcap (C)
  constexpr std::array<int, 4> subDet{97,98, 99, 100};
  std::array<int, 4> mrodCounter{make_array<int,4>(0)};
  std::array<int, 4> csmCounter{make_array<int,4>(0)};
  constexpr int mrodMax = 40;
  
  nlohmann::json allChannels{};
  for (const MuonGMR4::MdtReadoutElement* reEle : reEles) {
      const int detIndex = 2*(!reEle->isBarrel()) + (reEle->stationEta() < 0);
      const int mrod = (++mrodCounter[detIndex]);
      if (mrod ==mrodMax) {
          mrodCounter[detIndex] = 0;
          (++csmCounter[detIndex]);
      }

      const MdtMezzanineCard& card {reEle->numLayers() == 4 ? mapping4Layers: mapping3Layers};
      int tdc{0};
      
      for (uint tube =1; tube<= reEle->numTubesInLay(); tube+=card.numTubesPerLayer()) {
           nlohmann::json cablingChannel{};
           cablingChannel["station"] = m_idHelperSvc->stationNameString(reEle->identify());
           cablingChannel["eta"] = reEle->stationEta();
           cablingChannel["phi"] = reEle->stationPhi();
           cablingChannel["ml"] = reEle->multilayer();
           cablingChannel["subDet"] = subDet[detIndex];
           cablingChannel["csm"] = csmCounter[detIndex];
           cablingChannel["mrod"] = mrod;
           cablingChannel["tdcId"] = (tdc++);
           cablingChannel["mezzId"] = static_cast<int>(card.id());
           cablingChannel["tubeZero"] = tube;
           allChannels.push_back(cablingChannel);
      }
  }
  std::ofstream outStream{m_cablingJSON};
  if (!outStream.good()) {
    ATH_MSG_FATAL("Failed to create JSON file "<<m_cablingJSON);
    return StatusCode::FAILURE;
  }
  outStream<<allChannels.dump(2)<<std::endl;
  return StatusCode::SUCCESS;
}

