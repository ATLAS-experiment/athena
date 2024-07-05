/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "RpcToyCablingJsonDumpAlg.h"

#include "MuonCablingData/NrpcCablingData.h"

#include "nlohmann/json.hpp"
#include <fstream>



RpcToyCablingJsonDumpAlg::RpcToyCablingJsonDumpAlg(const std::string& name, ISvcLocator* pSvcLocator):
    AthAlgorithm{name, pSvcLocator} {}

StatusCode RpcToyCablingJsonDumpAlg::initialize() {
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(detStore()->retrieve(m_detMgr));
    if (!m_idHelperSvc->hasRPC()) {
        ATH_MSG_FATAL("You can't write rpc cablings without rpc detectors? ");
        return StatusCode::FAILURE;
    }

    m_BIL_stIdx = m_idHelperSvc->rpcIdHelper().stationNameIndex("BIL");
    return StatusCode::SUCCESS;
}
StatusCode RpcToyCablingJsonDumpAlg::execute() {
  std::ofstream outStream{m_cablingJSON};
  if (!outStream.good()) {
    ATH_MSG_FATAL("Failed to create JSON file "<<m_cablingJSON);
    return StatusCode::FAILURE;
  }
  
  std::vector<const MuonGMR4::RpcReadoutElement*> reEles{m_detMgr->getAllRpcReadoutElements()};

  /// Subdetector identifiers (0x65 for chambers on the posivie side and 0x66 for chambers on the other side
  constexpr int subDetA = 0x65;
  constexpr int subDetB = 0x66;
  constexpr unsigned int nStripsPerTdc = 64;

  unsigned int tdcSecA{0}, tdcSecC{0};

  nlohmann::json overallMap{};

  constexpr int16_t measPhiBit = NrpcCablingData::measPhiBit;
  constexpr int16_t stripSideBit = NrpcCablingData::stripSideBit;
  for (const MuonGMR4::RpcReadoutElement* reEle : reEles) {
      const unsigned int subDet = reEle->stationEta() > 0 ? subDetA : subDetB;
      const unsigned int tdcSec = reEle->stationEta() > 0 ? (++tdcSecA) : (++tdcSecC);
      unsigned int tdc{1};
      for (unsigned int gasGap =1 ; gasGap <= reEle->nGasGaps(); ++gasGap){
        for (int doubletPhi = reEle->doubletPhi(); doubletPhi <= reEle->doubletPhiMax(); ++doubletPhi) {
            for (bool measPhi: {false, true}) {
              const unsigned int nStrips = (measPhi ? reEle->nPhiStrips() : reEle->nEtaStrips())+1;
              const unsigned int nTdcStrips = (nStrips % nStripsPerTdc ? 1 : 0) + 
                                              (nStrips - (nStrips % nStripsPerTdc)) / nStripsPerTdc;  
                for (bool side : {false, true}) {
                    if (side && reEle->stationName() != m_BIL_stIdx) {
                      /// Do not create side cablings for non BIL stations
                        continue;
                    }
                    unsigned int measPhiSide = (side * stripSideBit) | (measPhi * measPhiBit);
                    for (unsigned int strip = 0; strip < nTdcStrips; ++strip) {
                        nlohmann::json cablingChannel{};
                        cablingChannel["station"] = m_idHelperSvc->stationNameString(reEle->identify());
                        cablingChannel["eta"] = reEle->stationEta();
                        cablingChannel["phi"] = reEle->stationPhi();
                        cablingChannel["doubletR"] = reEle->doubletR();
                        cablingChannel["doubletZ"] = reEle->doubletZ();
                        cablingChannel["doubletPhi"] = doubletPhi;                        
                        cablingChannel["gasGap"] = gasGap;
                        cablingChannel["measPhi"] = measPhiSide;

                        cablingChannel["subDetector"] = subDet;
                        cablingChannel["tdcSector"] = tdcSec;
                        cablingChannel["firstStrip"] = (1 + nStripsPerTdc*strip);
                        cablingChannel["lastStrip"] = nStripsPerTdc*(strip +1);
                        cablingChannel["firstTdcChan"] = 1;
                        cablingChannel["lastTdcChan"] = nStripsPerTdc;
                        cablingChannel["tdc"] = (++tdc);

                        overallMap.push_back(cablingChannel);
                    }
                }
            }
        }
     }
  }
  outStream<<overallMap.dump(2)<<std::endl;
  return StatusCode::SUCCESS;
}

