/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "RPCPatternMatching.h"

#include "MuonReadoutGeometryR4/MuonReadoutElement.h"
#include <MuonIdHelpers/IdentifierByDetElSorter.h>
#include "PathResolver/PathResolver.h"
#include "L1MuonS1RPC/RPCPatternUtils.h"

#include <fstream>

namespace L1Muon {

StatusCode RPCPatternMatching::initialize() {
  ATH_MSG_INFO("Initializing " << name() << "...");

  ATH_CHECK(m_keyRpcRdo.initialize());
  ATH_CHECK(m_cablingKey.initialize());
  ATH_CHECK(m_idHelperSvc.retrieve());
  ATH_CHECK(m_geoCtxKey.initialize());
  ATH_CHECK(detStore()->retrieve(m_detMgr));

  /// container of output trigger candidates
  ATH_CHECK(m_outputCandKey.initialize());

  // Initialize patterns
  if (!initializePatterns()) {
    ATH_MSG_FATAL("Failed to initialize patterns.");
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

StatusCode RPCPatternMatching::execute(const EventContext& ctx) const {
  ATH_MSG_DEBUG("Executing " << name() << "...");

  // Retrieve the input containers
  const xAOD::NRPCRDOContainer* rdoContainer{nullptr};
  ATH_CHECK(SG::get(rdoContainer, m_keyRpcRdo, ctx));

  const Muon::RpcCablingMap* cablingMap{nullptr};
  ATH_CHECK(SG::get(cablingMap, m_cablingKey, ctx));

  // Get ActsGeometryContext
  const ActsTrk::GeometryContext* gctxHandle{};
  ATH_CHECK(SG::get(gctxHandle, m_geoCtxKey, ctx));
  const ActsTrk::GeometryContext& geoCtx = *gctxHandle;

  /// output container for candidates
  SG::WriteHandle<xAOD::L1RPCCandDataContainer> outputCandHandle(m_outputCandKey, ctx);
  ATH_CHECK(outputCandHandle.record(std::make_unique<xAOD::L1RPCCandDataContainer>(), std::make_unique<xAOD::L1RPCCandDataAuxContainer>()));
  auto outputCands = outputCandHandle.ptr();

  // Get strip IDs for PatternMatching from SDOs/RDOs
  std::vector<uint32_t> stripIDs;
  std::map<uint32_t, uint32_t> strip_coordinates;
  std::map<uint32_t, uint32_t> strip_etabin;

  // sorting RDOs by offline Identifier to easily access both strip sides --> we can compute phi coordinate for BI strips
  std::map<Identifier, RdoPairs, Muon::IdentifierByDetElSorter> sortedRdos{Muon::IdentifierByDetElSorter{m_idHelperSvc.get()}};
  std::map<Identifier, Muon::RpcCablingData> sortedCabling;
  for (const xAOD::NRPCRDO* rdo : *rdoContainer){
      /* cabling data conversion */
      Muon::RpcCablingData cabling{};
      cabling.subDetector = rdo->subdetector();
      cabling.boardSector = rdo->boardsector();
      cabling.board = rdo->board();
      cabling.channelId = rdo->channel();

      if (!cablingMap->getOfflineId(cabling, msgStream())){
          ATH_MSG_FATAL("Cabling retrieval failed "<<cabling);
          return StatusCode::FAILURE;
      }

      Identifier offId{};
      if (!cablingMap->convert(cabling, offId)){
          ATH_MSG_FATAL("Cabling conversion failed "<<cabling);
          return StatusCode::FAILURE;
      }

      const bool stripSide = cabling.stripSide();
      sortedRdos[offId][stripSide] = rdo;
      sortedCabling[offId] = cabling;
  }

  for (const auto& [offId, rdoPairs] : sortedRdos) {
    Muon::RpcCablingData cabling = sortedCabling[offId];
    getStripIdforAlg(rdoPairs, stripIDs, strip_coordinates, strip_etabin, cabling, offId, geoCtx);
  }

  // check for the required strip coincidence within each RPC layer
  if (!checkStripCoincidence(stripIDs)) {
    ATH_MSG_DEBUG("No required strip coincidence found, unable to form a valid muon candidate.");
    return StatusCode::SUCCESS;
  }
  // use BI strip coordinates as seed for pattern matching
  // to avoid multiple matches for the same eta bin, we store the already matched eta bins
  // in uint16_t identifiers with the following structure:
  // bits 0-4: eta bin (0-21)
  // bits 5-10: sector (0-63)
  std::map<uint16_t, std::vector<uint32_t>> usedEtaBins; // to store already matched eta bins
  std::map<uint16_t, std::vector<RPCMuonPattern>> matchedEtaPatterns; // to store matched eta patterns
  std::map<uint16_t, std::vector<RPCMuonPattern>> matchedPhiPatterns; // to store matched phi patterns
  std::map<uint16_t, std::vector<RPCMuonPattern>> matchedPatterns; // to store matched eta+phi patterns --> muon candidates

  // Check eta_bin from BI strips only to reduce combinatorics
  // First check if any BI strip is present, if not use BM1 strips as seed (if present)
  bool hasBIStrip = false;
  for (auto strip: stripIDs) {
    if ((RPCSLStripConverter::getStation(strip) == SLRPCStation::BI_BOR_BOM && RPCSLStripConverter::getMeasuresPhi(strip) == 0)) {
      hasBIStrip = true;
      break;
    }
  }

  for (auto strip: stripIDs) {
    bool isBIorBM1 = (RPCSLStripConverter::getStation(strip) == SLRPCStation::BI_BOR_BOM && RPCSLStripConverter::getMeasuresPhi(strip) == 0); // check if the strip is in the BI layer and is an eta strip
    if (!hasBIStrip) {
      isBIorBM1 = (RPCSLStripConverter::getStation(strip) == SLRPCStation::BM1 && RPCSLStripConverter::getMeasuresPhi(strip) == 0); // if no BI strip, check for BM1 eta strips
    }
    if (isBIorBM1) {
      ATH_MSG_DEBUG("Using BI strip ID: " << std::hex << strip << " with coord: " << std::dec << strip_etabin[strip] << " as seed for pattern matching");
      // Perform pattern matching using this BI strip as seed
      // We get the eta bin from the strip coordinates and the trigger sector from the strip ID
      uint16_t triggerSector = RPCSLStripConverter::getTriggerSector(strip); // extract trigger sector from strip ID
      uint32_t etaBin = strip_etabin[strip];

      std::vector<uint32_t>& binsInSector = usedEtaBins[triggerSector];
      
      if (Acts::rangeContainsValue(binsInSector, etaBin)) {
        ATH_MSG_DEBUG("Eta bin " << etaBin << " in sector " << triggerSector << " already matched, skipping to avoid duplicates.");
        continue; 
      }

      auto it_sector = m_etaPatterns.find(triggerSector);
      if (it_sector == m_etaPatterns.end()) {
        ATH_MSG_WARNING("No eta patterns found for sector: " << triggerSector);
        continue;
      }

      auto it_etabin = it_sector->second.find(static_cast<uint16_t>(etaBin));
      if (it_etabin == it_sector->second.end()) {
        ATH_MSG_WARNING("No eta patterns found for sector: " << triggerSector << " and eta bin: " << etaBin);
        continue;
      }

      for (const auto& pattern : it_etabin->second) {
        matchPattern(pattern, stripIDs, matchedEtaPatterns[triggerSector], RPCSLStripConverter::getTriggerSector(strip));
      }

      if (matchedEtaPatterns[triggerSector].empty()) {
        ATH_MSG_DEBUG("No eta patterns matched for sector: " << triggerSector << " and eta bin: " << etaBin);
        continue;
      } else {
        ATH_MSG_DEBUG("Matched " << matchedEtaPatterns[triggerSector].size() << " eta patterns for sector: " << triggerSector << " and eta bin: " << etaBin);
      }

      it_sector = m_phiPatterns.find(triggerSector);
      if (it_sector == m_phiPatterns.end()) {
        ATH_MSG_WARNING("No phi patterns found for sector: " << triggerSector);
        continue;
      }

      it_etabin = it_sector->second.find(static_cast<uint16_t>(etaBin));
      if (it_etabin == it_sector->second.end()) {
        ATH_MSG_WARNING("No phi patterns found for sector: " << triggerSector << " and eta bin: " << etaBin);
        continue;
      } 

      for (const auto& pattern : it_etabin->second) {
        matchPattern(pattern, stripIDs, matchedPhiPatterns[triggerSector], RPCSLStripConverter::getTriggerSector(strip));
      }

      if (matchedPhiPatterns[triggerSector].empty()) {
        ATH_MSG_DEBUG("No phi patterns matched for sector: " << triggerSector << " and eta bin: " << etaBin);
        continue;
      } else {
        ATH_MSG_DEBUG("Matched " << matchedPhiPatterns[triggerSector].size() << " phi patterns for sector: " << triggerSector << " and eta bin: " << etaBin);
      }

      for (const auto& etaPattern : matchedEtaPatterns[triggerSector]) {
        for (const auto& phiPattern : matchedPhiPatterns[triggerSector]) {
          matchEtaAndPhiPatterns(etaPattern, phiPattern, matchedPatterns[triggerSector]);
        }
      }
      usedEtaBins[triggerSector].push_back(etaBin); // mark this eta bin in this sector as matched
    }
  }

  // For now, we directly create final candidates
  if (!createCandidate(matchedPatterns, *outputCands)) {
    ATH_MSG_WARNING("Failed to create candidates from matched patterns.");
    return StatusCode::SUCCESS; // continue even if candidate creation fails
  }
  return StatusCode::SUCCESS;
}

void RPCPatternMatching::matchEtaAndPhiPatterns(const RPCMuonPattern& etaPattern, const RPCMuonPattern& phiPattern, std::vector<RPCMuonPattern>& matchedPatterns) const {
  // This function implements the logic to match a given eta and phi pattern together to form a muon candidate
  // The matching logic can be based on the compatibility of the eta and phi patterns (e.g. same sector, compatible coordinates, etc.)
  // For simplicity, we will just check if they belong to the same sector and if they have compatible eta bins

  if (etaPattern.getTriggerSector() != phiPattern.getTriggerSector()) {
    return; // patterns must be in the same sector to be matched
  }

  if (etaPattern.getEtaBin() != phiPattern.getEtaBin()) {
    return; // patterns must have the same eta bin to be matched
  }
  
  // combine the eta and phi patterns infos
  uint32_t combinedInfos = etaPattern.getInfos() | phiPattern.getInfos();

  // create a new pattern with the combined infos and add it to the matched patterns
  matchedPatterns.push_back(RPCMuonPattern(etaPattern, combinedInfos));

}
StatusCode RPCPatternMatching::loadPatternFile(const std::string& filePath, const std::string& patternType, PatternContainer& patternContainer) {
  std::ifstream file(filePath, std::ios::binary);
  if (!file) {
    ATH_MSG_ERROR("Failed to open " << patternType << " pattern file: " << filePath);
    return StatusCode::FAILURE;
  }

  // Read global triggerSector (sector ID)
  uint16_t sector_id;
  file.read(reinterpret_cast<char*>(&sector_id), sizeof(sector_id));
  if (!file) {
    ATH_MSG_ERROR("Failed to read sector ID from " << patternType << " pattern file: " << filePath);
    return StatusCode::FAILURE;
  }
  ATH_MSG_INFO("Reading " << patternType << " patterns for sector ID: " << sector_id);

  // Read patterns until EOF
  while (true) {
    uint32_t etaBin, numPatterns;
    file.read(reinterpret_cast<char*>(&etaBin), sizeof(etaBin));
    if (!file) break; // EOF
    file.read(reinterpret_cast<char*>(&numPatterns), sizeof(numPatterns));
    if (!file) break; // EOF

    // Apply eta-specific assertions
    if (patternType == "eta") {
      assert(etaBin < 22); // assuming a maximum of 22 eta bins (0-21)
      assert(numPatterns < 20000); // assuming a maximum of 20000 patterns per eta bin
    }

    std::vector<RPCMuonPattern> patterns;
    ATH_MSG_DEBUG("Reading " << numPatterns << " patterns for eta bin: " << etaBin);
    patterns.reserve(numPatterns);
    
    for (uint32_t i = 0; i < numPatterns; i++) {
      uint32_t bibm1, bm2bo, coordbibm1, coordbm2bo, infos, separator;
      file.read(reinterpret_cast<char*>(&bibm1), sizeof(bibm1));
      file.read(reinterpret_cast<char*>(&bm2bo), sizeof(bm2bo));
      file.read(reinterpret_cast<char*>(&coordbibm1), sizeof(coordbibm1));
      file.read(reinterpret_cast<char*>(&coordbm2bo), sizeof(coordbm2bo));
      file.read(reinterpret_cast<char*>(&infos), sizeof(infos));
      file.read(reinterpret_cast<char*>(&separator), sizeof(separator));

      if (!file) {
        ATH_MSG_ERROR("Incomplete pattern data in " << patternType << " file.");
        return StatusCode::FAILURE;
      }

      if (separator != 0xFFFFFFFF) {
        ATH_MSG_ERROR("Invalid separator " << std::hex << separator << " found in " << patternType << " patterns");
        return StatusCode::FAILURE;
      }

      if (!patternSanityCheck(RPCMuonPattern(sector_id, bibm1, bm2bo, coordbibm1, coordbm2bo, infos))) {
        ATH_MSG_WARNING("Pattern sanity check failed for " << patternType << " pattern with bibm1: " << std::hex << bibm1 << std::dec 
                      << ", bm2bo: " << std::hex << bm2bo << std::dec 
                      << ", coordbibm1: " << coordbibm1 
                      << ", coordbm2bo: " << coordbm2bo 
                      << ", infos: " << infos 
                      << " in sector " << sector_id 
                      << " and eta bin " << etaBin << ". Skipping this pattern.");
        continue;
      }

      // Construct RPCMuonPattern
      patterns.emplace_back(sector_id, bibm1, bm2bo, coordbibm1, coordbm2bo, infos);
    }
    
    patternContainer[static_cast<uint16_t>(sector_id)][static_cast<uint16_t>(etaBin)] = std::move(patterns);
    
    auto& loadedPatterns = patternContainer[static_cast<uint16_t>(sector_id)][static_cast<uint16_t>(etaBin)];
    ATH_MSG_DEBUG("Loaded " << loadedPatterns.size() << " / " << numPatterns << " " << patternType << " patterns for sector: " << sector_id << " and eta bin: " << etaBin);

    if (!loadedPatterns.empty()) {
      ATH_MSG_DEBUG("First pattern details - bibm1: " << std::hex << loadedPatterns.front().getBIBM1() << std::dec
                  << ", bm2bo: " << std::hex << loadedPatterns.front().getBM2BO() << std::dec 
                  << ", coordbibm1: " << loadedPatterns.front().getCoordBIBM1() 
                  << ", coordbm2bo: " << loadedPatterns.front().getCoordBM2BO() 
                  << ", infos: " << loadedPatterns.front().getInfos());

      ATH_MSG_DEBUG("Last pattern details - bibm1: " << std::hex << loadedPatterns.back().getBIBM1() << std::dec 
                  << ", bm2bo: " << std::hex << loadedPatterns.back().getBM2BO() << std::dec 
                  << ", coordbibm1: " << loadedPatterns.back().getCoordBIBM1() 
                  << ", coordbm2bo: " << loadedPatterns.back().getCoordBM2BO() 
                  << ", infos: " << loadedPatterns.back().getInfos());
    } else {
      ATH_MSG_WARNING("Pattern vector is empty! Skipping details printout.");
    }
  }
  
  return StatusCode::SUCCESS;
}

StatusCode RPCPatternMatching::initializePatterns() {
  ATH_MSG_INFO("Initializing patterns...");

  for (int sec = 0; sec < 64; ++sec) { // 64 trigger sectors
    std::string etaFileName = std::format("dev/L1MuonS1RPCPattern/v1/L1MuonRPCPattern_Eta_Sector_{:}.bin", sec);
    std::string phiFileName = std::format("dev/L1MuonS1RPCPattern/v1/L1MuonRPCPattern_Phi_Sector_{:}.bin", sec);
    std::string etaFilePath = PathResolverFindCalibFile(etaFileName);
    std::string phiFilePath = PathResolverFindCalibFile(phiFileName);

    if (etaFilePath.empty() || phiFilePath.empty()) {
      ATH_MSG_ERROR("Could not locate pattern files for sector " << sec);
      return StatusCode::FAILURE;
    }

    if (loadPatternFile(etaFilePath, "eta", m_etaPatterns).isFailure()) return StatusCode::FAILURE;
    if (loadPatternFile(phiFilePath, "phi", m_phiPatterns).isFailure()) return StatusCode::FAILURE;
  }

  ATH_MSG_INFO("Pattern initialization complete.");
  return StatusCode::SUCCESS;
}

int RPCPatternMatching::getVirtualBIPhiStripNumber(float loc_pos_along_BIstrip, const MuonGMR4::RpcReadoutElement& readoutElement) const {
    return RPCPatternUtils::getVirtualBIPhiStripNumber(loc_pos_along_BIstrip, readoutElement);
}

uint32_t RPCPatternMatching::getVirtualBIPhiStripCoordinate(const Amg::Vector3D& stripPos, float loc_pos_along_BIstrip) const {
    return RPCPatternUtils::getVirtualBIPhiStripCoordinate(stripPos, loc_pos_along_BIstrip);
}

uint32_t RPCPatternMatching::getSLStripId(bool side, int sector, int doubletPhi, Muon::MuonStationIndex::StIndex RPClayer, int doubletR, int stationEta, int doubletZ, int stripNumber, bool measuresPhi) const {
    return RPCPatternUtils::getSLStripId(side, sector, doubletPhi, 
                                        RPClayer, doubletR, stationEta,
                                        doubletZ, stripNumber, measuresPhi);
}

uint32_t RPCPatternMatching::getBMBOPhiStripCoordinate(const Amg::Vector3D& stripPos) const {
    return RPCPatternUtils::getBMBOPhiStripCoordinate(stripPos);
}

void RPCPatternMatching::getStripIdforAlg(const RdoPairs& rdo_pair, std::vector<uint32_t>& stripIDs, std::map<uint32_t, uint32_t>& strip_coordinates, std::map<uint32_t, uint32_t>& strip_etabin,
                                               const Muon::RpcCablingData& cabling, const Identifier& offId, const ActsTrk::GeometryContext& geoCtx) const {
    return RPCPatternUtils::getStripIdforAlg(rdo_pair, stripIDs, strip_coordinates, strip_etabin, cabling, offId, geoCtx, m_idHelperSvc.get(), m_detMgr);
}

void RPCPatternMatching::matchPattern(const RPCMuonPattern& pattern, const std::vector<uint32_t>& stripIDs, 
                                            std::vector<RPCMuonPattern>& matchedPatterns, const uint16_t& triggerSector) const {
  // This function implements the logic to match a given pattern against the provided strip IDs                           
  bool match = false;
  auto stripBI = pattern.getStripBI() | static_cast<uint32_t>(triggerSector) << 16;
  auto stripBM1 = pattern.getStripBM1() | static_cast<uint32_t>(triggerSector) << 16;
  auto stripBM2 = pattern.getStripBM2() | static_cast<uint32_t>(triggerSector) << 16;
  auto stripBO = pattern.getStripBO() | static_cast<uint32_t>(triggerSector) << 16;
  auto priority = pattern.getPriority();

  // different case depending on the pattern priority
  if (priority == 5) { //BI-BM2-BO
    match = Acts::rangeContainsValue(stripIDs, stripBI) &&
            Acts::rangeContainsValue(stripIDs, stripBM2) &&
            Acts::rangeContainsValue(stripIDs, stripBO);
  } else if (priority == 4) { //BI-BM1-BO
    match = Acts::rangeContainsValue(stripIDs, stripBI) &&
            Acts::rangeContainsValue(stripIDs, stripBM1) &&
            Acts::rangeContainsValue(stripIDs, stripBO);
  } else if (priority == 3) { //BM1-BM2-BO
    match = Acts::rangeContainsValue(stripIDs, stripBM1) &&
            Acts::rangeContainsValue(stripIDs, stripBM2) &&
            Acts::rangeContainsValue(stripIDs, stripBO);
  } else if (priority == 2) { //BI-BM1-BM2
    match = Acts::rangeContainsValue(stripIDs, stripBI) &&
            Acts::rangeContainsValue(stripIDs, stripBM1) &&
            Acts::rangeContainsValue(stripIDs, stripBM2);
  } else if (priority == 1) { //BI-BO
    match = Acts::rangeContainsValue(stripIDs, stripBI) &&
            Acts::rangeContainsValue(stripIDs, stripBO);
  } else {
    ATH_MSG_WARNING("Unknown pattern priority: " << priority);
  }

  if (match) {
    matchedPatterns.push_back(pattern);
    ATH_MSG_DEBUG("Pattern matched.");
    // pattern.print();
  }
}

bool RPCPatternMatching::checkStripCoincidence(std::vector<uint32_t>& stripIDs) const {
  // This function checks for the required strip coincidence within each RPC layer
  // (2/3 majority logic for BI and 1/2 logic for BM and BO) and verifies that there
  // is a valid muon candidate to process.

  // Loop over all possible trigger sectors (0-63) to check for coincidences
  bool validCandidate = false;
  std::vector<uint32_t> stripIDsAfterStripCoincidence;

  std::sort(stripIDs.begin(), stripIDs.end());
  for (uint16_t triggerSector = 0; triggerSector < 64; triggerSector++) {
    std::vector<uint32_t> stripIDsInSector;
    for (const auto& stripID : stripIDs) {
      if (RPCSLStripConverter::getTriggerSector(stripID) == triggerSector) {
        stripIDsInSector.push_back(stripID);
      }
    }

    if (stripIDsInSector.empty()) continue; // No strips for this triggerSector, skip to the next one

    // Check for coincidences in the current sector
    bool hasBI[2] = {false, false};   // [0] = measuresEta, [1] = measuresPhi
    bool hasBM1[2] = {false, false};  // [0] = measuresEta, [1] = measuresPhi
    bool hasBM2[2] = {false, false};  // [0] = measuresEta, [1] = measuresPhi
    bool hasBO[2] = {false, false};   // [0] = measuresEta, [1] = measuresPhi

    std::sort(stripIDsInSector.begin(), stripIDsInSector.end());
    std::vector<uint32_t> stripID_unique = stripIDsInSector; // create a copy of the original vector to keep only unique strip IDs that pass the coincidence requirement
    stripID_unique.erase(std::unique(stripID_unique.begin(), stripID_unique.end()), stripID_unique.end());

    for (int i = 0; i < int(stripID_unique.size()); ++i) {
      if (RPCSLStripConverter::getStation(stripID_unique[i]) == SLRPCStation::BI_BOR_BOM) {
        uint32_t current_strip = stripID_unique[i];
        uint32_t next_strip = current_strip + 0x2;
        uint32_t prev_strip = current_strip - 0x2;
        int count = std::count(stripIDsInSector.begin(), stripIDsInSector.end(), current_strip) +
                    std::count(stripIDsInSector.begin(), stripIDsInSector.end(), next_strip) +
                    std::count(stripIDsInSector.begin(), stripIDsInSector.end(), prev_strip);
        if (count < 2) {
          stripID_unique.erase(std::remove(stripID_unique.begin(), stripID_unique.end(), stripID_unique[i]), stripID_unique.end());
          i--; // Adjust index after erasing
        }
        else {
          if (!hasBI[stripID_unique[i]& 0x1]) {
            hasBI[stripID_unique[i]& 0x1] = true;
          }
        }
      }
      else if (RPCSLStripConverter::getStation(stripID_unique[i]) == SLRPCStation::BM1) {
        hasBM1[stripID_unique[i] & 0x1] = true;
      } else if (RPCSLStripConverter::getStation(stripID_unique[i]) == SLRPCStation::BM2) {
        hasBM2[stripID_unique[i] & 0x1] = true;
      } else if (RPCSLStripConverter::getStation(stripID_unique[i]) == SLRPCStation::BO) {
        // BORBOM are treated as BI strips
        // if (some condition){
        //   // Do something
        // } else {
          hasBO[stripID_unique[i] & 0x1] = true;
        // }
      }
    }
    stripIDsAfterStripCoincidence.insert(stripIDsAfterStripCoincidence.end(), stripID_unique.begin(), stripID_unique.end());
    
    bool valid_eta_cand = (hasBI[0] + hasBM1[0] + hasBM2[0] + hasBO[0] > 2) || (hasBI[0] + hasBO[0] == 2);
    bool valid_phi_cand = (hasBI[1] + hasBM1[1] + hasBM2[1] + hasBO[1] > 2) || (hasBI[1] + hasBO[1] == 2);
    if (valid_eta_cand && valid_phi_cand) {
      validCandidate = true;
    }
  }

  stripIDs = std::move(stripIDsAfterStripCoincidence); // Update the original stripIDs vector with the filtered results
  return validCandidate;
 }

StatusCode RPCPatternMatching::createCandidate(const std::map<uint16_t, std::vector<RPCMuonPattern>>& matchedPatterns, xAOD::L1RPCCandDataContainer& outputCand) const {
  // This function creates the final muon candidates from the matched patterns
  // and stores them in the output container
  static const std::map<int, int> th_map = {{4, 0}, {5, 1}, {6, 2}, {7, 3}, {8, 4}, {9, 5}, {10, 6}, \
                                            {12, 7}, {15, 8}, {18, 9}, {20, 10}, {25, 11}, {30, 12}, \
                                            {40, 13}, {80, 14}, {100, 15}};

  // Provide up to two candidates per sector                                          
  for (const auto & [sector, patterns] : matchedPatterns){
    int cand_count = 0;
    //Select one pattern per eta bin (the one with the highest priority and then highest pt)
    std::map<uint32_t, RPCMuonPattern> best_patterns;
    getBestCandidatesPerEtaBin(patterns, best_patterns);

    for (const auto& [etaBin, pattern] : best_patterns) {
      if (cand_count >=2) {
        ATH_MSG_DEBUG("Maximum number of candidates reached for sector " << sector << ", skipping remaining patterns.");
        break; // limit to 2 candidates per sector
      }
      cand_count++;
      uint16_t subdetectorId = pattern.getTriggerSector() > 31 ? 0x65 : 0x66; // 0x65 for positive eta, 0x66 for negative eta
      uint16_t sectorId = pattern.getTriggerSector();
      uint16_t bcTag = 0; // placeholder, to be set properly


      xAOD::L1RPCCandData* newCand = outputCand.push_back(std::make_unique<xAOD::L1RPCCandData>());

      newCand->initialize(subdetectorId, sectorId, bcTag);
      newCand->setL1Eta(pattern.getEtaFloat());
      newCand->setL1Phi(pattern.getPhiFloat());
      newCand->setL1Pt(pattern.getPtFloat());
      newCand->setL1Threshold(pattern.getMaxThreshold());
      newCand->setL1CandCharge(pattern.getCharge());
      newCand->setCoinType(pattern.getPriority());

      std::array<float, 4> zPos{
                            pattern.getCoordBIFloat(),
                            pattern.getCoordBM1Float(),
                            pattern.getCoordBM2Float(),
                            pattern.getCoordBOFloat()};

      newCand->setL1ZPos(zPos);

    } 
  }

  return StatusCode::SUCCESS;
}

void RPCPatternMatching::getBestCandidatesPerEtaBin(const std::vector<RPCMuonPattern>& patterns, 
                                                    std::map<uint32_t, RPCMuonPattern>& best_patterns) const {
  // This function selects the best candidate per eta bin based on priority and pt
  // TO DO: review the selection criteria if SL firmware logic is different
  // 1) Group patterns by eta bin
  std::map<uint32_t, std::vector<RPCMuonPattern>> patterns_per_eta;
  for (const auto& pattern : patterns) {
    uint32_t etaBin = pattern.getEtaBin();
    patterns_per_eta[etaBin].push_back(pattern);
  }
  // 2) Sort patterns within each eta bin by priority and pt
  for (auto& [etaBin, eta_patterns] : patterns_per_eta) {
    std::sort(eta_patterns.begin(), eta_patterns.end(),
              [](const RPCMuonPattern& a, const RPCMuonPattern& b) {
                if (a.getPriority() != b.getPriority()) {
                  return a.getPriority() > b.getPriority(); // higher priority first
                }
                return a.getPt() > b.getPt(); // higher pt first
              });
    // 3) Select the best pattern for this eta bin
    best_patterns.emplace(etaBin, eta_patterns.front());
  }
}

bool RPCPatternMatching::patternSanityCheck(const RPCMuonPattern& pattern) const {
  // This function checks the sanity of the pattern data read from the files
  // For example, it can check that the strip IDs are within expected ranges,
  // that the priority is valid, etc.

  if (pattern.getPriority() < 1 || pattern.getPriority() > 5) {
    ATH_MSG_DEBUG("Invalid pattern priority: " << pattern.getPriority());
    return false;
  }
  if (pattern.getStripBI() > 0x4000) {
    ATH_MSG_DEBUG("Invalid BI strip ID: " << std::hex << pattern.getStripBI() << std::dec);
    return false;
  }
  if (pattern.getStripBM1() > 0x8000 || pattern.getStripBM1() < 0x4000) {
    ATH_MSG_DEBUG("Invalid BM1 strip ID: " << std::hex << pattern.getStripBM1() << std::dec);
    return false;
  }
  if (pattern.getStripBM2() > 0xC000 || pattern.getStripBM2() < 0x8000) {
    ATH_MSG_DEBUG("Invalid BM2 strip ID: " << std::hex << pattern.getStripBM2() << std::dec);
    return false;
  }
  if (pattern.getStripBO() > 0x10000 || pattern.getStripBO() < 0xC000) {
    ATH_MSG_DEBUG("Invalid BO strip ID: " << std::hex << pattern.getStripBO() << std::dec);
    return false;
  }
  if (pattern.getEtaBin() > 22) {
    ATH_MSG_DEBUG("Invalid eta bin: " << pattern.getEtaBin());
    return false;
  }

  return true;
}

} // namespace L1Muon
