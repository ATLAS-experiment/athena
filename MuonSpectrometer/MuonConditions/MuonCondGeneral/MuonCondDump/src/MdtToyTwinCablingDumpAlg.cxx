#include "MdtToyTwinCablingDumpAlg.h"
#include <algorithm>
#include <vector>

#include "nlohmann/json.hpp"
#include <fstream>



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
StatusCode MdtToyTwinCablingDumpAlg::execute() {

  std::vector<const MuonGMR4::MdtReadoutElement*> reEles{m_detMgr->getAllMdtReadoutElements()};

  nlohmann::json allChannels{};

  for(const MuonGMR4::MdtReadoutElement* &reEl : reEles){
    Identifier id = reEl->identify();
    if(std::find(m_stationsToTwin.begin(), m_stationsToTwin.end(), m_idHelperSvc->stationNameString(id)) == m_stationsToTwin.end()) continue; // did not request twin tubes in this station
    for( uint iLayer{1}; iLayer <= reEl->numLayers(); iLayer++){
        std::vector<uint> hasTwin;
        hasTwin.reserve(reEl->numTubesInLay());
        for(uint iTube{1}; iTube<=reEl->numTubesInLay();iTube++){
            if(std::find(hasTwin.begin(), hasTwin.end(), iTube)!= hasTwin.end()) continue; // this tube already has a sibling
            uint twinTubeNumber =  iTube +2;

            bool isValid{false};
            m_idHelperSvc->mdtIdHelper().channelID(id, m_idHelperSvc->mdtIdHelper().multilayer(id), iLayer, iTube, isValid);
            if(!isValid) continue;
            m_idHelperSvc->mdtIdHelper().channelID(id, m_idHelperSvc->mdtIdHelper().multilayer(id), iLayer, twinTubeNumber, isValid);
            if(!isValid) continue;

            nlohmann::json twinPair{};
            hasTwin.push_back(iTube);
            hasTwin.push_back(twinTubeNumber);

            twinPair["station"] =   m_idHelperSvc->stationNameString(id);
            twinPair["eta"] = m_idHelperSvc->stationEta(id);
            twinPair["phi"] = m_idHelperSvc->stationPhi(id);
            twinPair["ml"] = m_idHelperSvc->mdtIdHelper().multilayer(id);

            twinPair["layer"] = iLayer;
            twinPair["tube"] = iTube;
            twinPair["layerTwin"] = iLayer;
            twinPair["tubeTwin"] = twinTubeNumber;


            allChannels.push_back(twinPair);

       }
    }
  }

  std::ofstream outStream{m_cablingJSON};
  if(!outStream.good()){
    ATH_MSG_FATAL("Failed to create JSON " << m_cablingJSON);
    return StatusCode::FAILURE;
  }

  outStream<<allChannels.dump(-1)<<std::endl;

  return StatusCode::SUCCESS;
}