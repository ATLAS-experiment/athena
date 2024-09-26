/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "MdtToyTwinCablingDumpAlg.h"
#include <algorithm>
#include <vector>
#include "MuonCablingData/HedgehogBoard.h"
#include "MuonCablingData/TwinTubeMap.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <format>

MdtToyTwinCablingDumpAlg::MdtToyTwinCablingDumpAlg(const std::string& name, ISvcLocator* pSvcLocator):
    AthAlgorithm{name, pSvcLocator} {}

StatusCode MdtToyTwinCablingDumpAlg::initialize() {
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(detStore()->retrieve(m_detMgr));
    if (!m_idHelperSvc->hasMDT()) {
        ATH_MSG_FATAL("You can't write mdt twin cablings without mdt detectors? ");
        return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
}
bool MdtToyTwinCablingDumpAlg::equipREwithTwins(const Identifier& detElId) const {
    if(std::ranges::find(m_stationsToTwin, m_idHelperSvc->stationNameString(detElId))!= m_stationsToTwin.end()) {
        return true;
    }
    const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};
    std::string mlName{std::format("{:}{:d}{:}{:02}M{:d}", 
                                   m_idHelperSvc->stationNameString(detElId),
                                   std::abs(m_idHelperSvc->stationEta(detElId)),
                                   (m_idHelperSvc->stationEta(detElId) > 0 ? 'A' : 'C'),
                                   m_idHelperSvc->sector(detElId),
                                   idHelper.multilayer(detElId))};
    
    ATH_MSG_DEBUG("Test "<<mlName<<", "<<m_detElIdToTwin);
    if(std::ranges::find(m_detElIdToTwin, mlName)!= m_detElIdToTwin.end()) {
        return true;
    }

    return m_detElIdToTwin.value().empty() && m_stationsToTwin.value().empty();
}
StatusCode MdtToyTwinCablingDumpAlg::execute() {
  using Mapping = Muon::HedgehogBoard::Mapping;
  using HedgeHogBoardPtr = Muon::HedgehogBoard::HedgehogBoardPtr;
  auto createMap = [this](const uint8_t tubeLayers, const uint16_t id) {
      Mapping dummyMap{make_array<uint8_t, 24>(-1)};
      const Muon::HedgehogBoard dummyBoard{dummyMap, tubeLayers, id};
      for (uint8_t layer = 1 ; layer<= dummyBoard.numTubeLayers(); ++layer) {
          for (uint8_t tube = 1; tube<= dummyBoard.numTubesPerLayer(); ++ tube) {
            const uint8_t thisPin = dummyBoard.pinNumber(layer, tube);
            const uint8_t otherTube = dummyBoard.numTubeLayers() == 3 || tube != 2 ? tube +2 : 5;       
            const uint8_t otherPin = dummyBoard.pinNumber(layer, otherTube);
            ATH_MSG_DEBUG("Layer "<<static_cast<int>(layer)<<", "<<static_cast<int>(tube)<<" -> pin: "
                          <<static_cast<int>(thisPin)<<" <=> tube: "<<static_cast<int>(tube + 2)<<", pin: "
                          <<static_cast<int>(otherPin));
            if (dummyMap[thisPin] < dummyMap.size()) {
              continue;
            }            
            dummyMap[otherPin] = thisPin;
            dummyMap[thisPin] = otherPin;
          }
      }
      auto board = std ::make_unique<Muon::HedgehogBoard>(dummyMap, tubeLayers, id);
      ATH_MSG_INFO("Created new hedgehog board "<<std::endl<<(*board));
      return board;
  };
  HedgeHogBoardPtr threeLayBoard{createMap(3,1)}, fourLayBoard{createMap(4,2)};

  std::vector<const MuonGMR4::MdtReadoutElement*> reEles{m_detMgr->getAllMdtReadoutElements()};

  nlohmann::json payload{};
  Muon::TwinTubeMap twinTubes{m_idHelperSvc.get()};
  for(const MuonGMR4::MdtReadoutElement* reEl : reEles){
    const Identifier id = reEl->identify();
    if (!equipREwithTwins(id)) {
      continue;
    }
    const HedgeHogBoardPtr& pickMe{reEl->numLayers() == 3 ? threeLayBoard : fourLayBoard};
    uint16_t boardCounter{0};
    std::vector<uint16_t> connectedBoards{};
    for (unsigned iTube{1}; iTube<= reEl->numTubesInLay()  + (reEl->numTubesInLay() % pickMe->numTubesPerLayer()); 
                  iTube+=pickMe->numTubesPerLayer()) {
        ATH_CHECK(twinTubes.addHedgeHogBoard(id, pickMe, boardCounter++));
        connectedBoards.push_back(pickMe->boardId());
    }
    nlohmann::json twinPair{};
    twinPair["station"] =   m_idHelperSvc->stationNameString(id);
    twinPair["eta"] = m_idHelperSvc->stationEta(id);
    twinPair["phi"] = m_idHelperSvc->stationPhi(id);
    twinPair["ml"] = m_idHelperSvc->mdtIdHelper().multilayer(id);
    twinPair["mountedBoards"] = connectedBoards;
    payload["TwinTubeMapping"].push_back(std::move(twinPair));
  }

  auto dumpHedgeHog = [&payload, this](const HedgeHogBoardPtr& board) {
      if (board.use_count() < 2) {
        ATH_MSG_INFO("The board "<<std::endl<<(*board)<<" has not been used "<<board.use_count());
        return;
      }
      nlohmann::json hedge{};
      hedge["boardId"] = board->boardId();
      hedge["nTubeLayers"] = board->numTubeLayers();
      hedge["hedgeHogPins"] = board->data();
      if (board->hasHVDelayTime()) {
        hedge["hvDelayTime"] = board->hvDelayTime();
      }
      payload["HedgehogBoards"].push_back(std::move(hedge));
  };
  dumpHedgeHog(threeLayBoard);
  dumpHedgeHog(fourLayBoard);

  std::ofstream outStream{m_cablingJSON};
  if(!outStream.good()){
    ATH_MSG_FATAL("Failed to create JSON " << m_cablingJSON);
    return StatusCode::FAILURE;
  }

  outStream<<payload.dump(m_spacing)<<std::endl;

  return StatusCode::SUCCESS;
}