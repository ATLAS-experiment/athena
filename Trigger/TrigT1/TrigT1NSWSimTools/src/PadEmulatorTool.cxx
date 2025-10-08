/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "PadEmulatorTool.h"

namespace NSWL1 {
  PadEmulatorTool::PadEmulatorTool(const std::string& type, const std::string& name, const IInterface* parent) :
  base_class(type,name,parent) {}

  StatusCode PadEmulatorTool::initialize() {
    ATH_MSG_INFO(           m_isMC.name()                  << " --> " << (m_isMC                  ? "[True]" : "[False]")
                 << ", " << m_doNtuple.name()              << " --> " << (m_doNtuple              ? "[True]" : "[False]")
                 << ", " << m_stretch.name()               << " --> " <<  m_stretch.value()
                 << ", " << m_ignoreBCIDs.name()           << " --> " << (m_ignoreBCIDs           ? "[True]" : "[False]")
                 << ", " << m_lastPhiOnly.name()           << " --> " << (m_lastPhiOnly           ? "[True]" : "[False]")
                 << ", " << m_bandIDPriorityEncode.name()  << " --> " << (m_bandIDPriorityEncode  ? "[True]" : "[False]")
                 << ", " << m_noDuplicates.name()          << " --> " << (m_noDuplicates          ? "[True]" : "[False]")
                 << ", " << m_triggerLogic.name()          << " --> " <<  m_triggerLogic.value()
                 << ", " << m_pattername_L.name()          << " --> " <<  m_pattername_L.value()
                 << ", " << m_pattername_S.name()          << " --> " <<  m_pattername_S.value()                        );

    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_detectorManagerKey.initialize());
    ATH_CHECK(m_sTGCDigitContainerKey.initialize());
    ATH_CHECK(m_dcsKey.initialize(!m_isMC));

    // Read pattern file for large and small sectors
    ATH_CHECK(loadPatterns(m_pattername_L.value()));
    ATH_CHECK(loadPatterns(m_pattername_S.value()));

    // Mask specific patterns according to known issues
    ATH_CHECK(maskPatterns());

    // Create ROB IDs
    ATH_CHECK(createRobIDs());

    // Do not allow debug ntuple in multi-threaded mode
    if(m_doNtuple and Gaudi::Concurrency::ConcurrencyFlags::numConcurrentEvents() > 1) {
      ATH_MSG_ERROR("DoNtuple is not possible in multi-threaded mode");
      return StatusCode::FAILURE;
    }

    ATH_MSG_DEBUG("Successfully initialized " << name() );
    return StatusCode::SUCCESS;
  }

  StatusCode PadEmulatorTool::attachBranches(MuonVal::MuonTesterTree &tree) {
    m_padTrigger_digits_sector = std::make_shared<MuonVal::VectorBranch<int> >(tree, "sTGC_PadTrigger_digits_sector");
    m_padTrigger_digits_eta = std::make_shared<MuonVal::VectorBranch<int> >(tree, "sTGC_PadTrigger_digits_eta");
    m_padTrigger_digits_phi = std::make_shared<MuonVal::VectorBranch<int> >(tree, "sTGC_PadTrigger_digits_phi");
    m_padTrigger_digits_multilayer = std::make_shared<MuonVal::VectorBranch<int> >(tree, "sTGC_PadTrigger_digits_multilayer");
    m_padTrigger_digits_gasgap = std::make_shared<MuonVal::VectorBranch<int> >(tree, "sTGC_PadTrigger_digits_gasgap");
    m_padTrigger_digits_source_id = std::make_shared<MuonVal::VectorBranch<uint32_t> >(tree, "sTGC_PadTrigger_digits_source_id");
    m_padTrigger_digits_pfeb = std::make_shared<MuonVal::VectorBranch<uint32_t> >(tree, "sTGC_PadTrigger_digits_pfeb");
    m_padTrigger_digits_pad_channel = std::make_shared<MuonVal::VectorBranch<uint32_t> >(tree, "sTGC_PadTrigger_digits_padchannel");
    m_padTrigger_digits_relbcid = std::make_shared<MuonVal::VectorBranch<uint32_t> >(tree, "sTGC_PadTrigger_digits_relbcid");
    m_padTrigger_TP_R_id_init = std::make_shared<MuonVal::VectorBranch<uint8_t> >(tree, "sTGC_PadTrigger_TP_R_id_init");
    m_padTrigger_TP_phi_id_init = std::make_shared<MuonVal::VectorBranch<uint8_t> >(tree, "sTGC_PadTrigger_TP_phi_id_init");
    m_padTrigger_TP_relbcid_init = std::make_shared<MuonVal::VectorBranch<uint32_t> >(tree, "sTGC_PadTrigger_TP_relbcid_init");
    m_padTrigger_TP_R_id = std::make_shared<MuonVal::VectorBranch<uint8_t> >(tree, "sTGC_PadTrigger_TP_R_id");
    m_padTrigger_TP_phi_id = std::make_shared<MuonVal::VectorBranch<uint8_t> >(tree, "sTGC_PadTrigger_TP_phi_id");
    m_padTrigger_TP_relbcid = std::make_shared<MuonVal::VectorBranch<uint32_t> >(tree, "sTGC_PadTrigger_TP_relbcid");
    m_padTrigger_wheel = std::make_shared<MuonVal::VectorBranch<char> >(tree, "sTGC_PadTrigger_wheel");
    m_padTrigger_sector = std::make_shared<MuonVal::VectorBranch<uint32_t> >(tree, "sTGC_PadTrigger_sector");
    m_padTrigger_hitmask  = std::make_shared<MuonVal::VectorBranch<uint32_t> >(tree, "sTGC_PadTrigger_hitmask");
    m_padTrigger_efficiency = std::make_shared<MuonVal::VectorBranch<uint32_t> >(tree, "sTGC_PadTrigger_efficiency");

    tree.addBranch(m_padTrigger_digits_sector);
    tree.addBranch(m_padTrigger_digits_eta);
    tree.addBranch(m_padTrigger_digits_phi);
    tree.addBranch(m_padTrigger_digits_multilayer);
    tree.addBranch(m_padTrigger_digits_gasgap);
    tree.addBranch(m_padTrigger_digits_source_id);
    tree.addBranch(m_padTrigger_digits_pfeb);
    tree.addBranch(m_padTrigger_digits_pad_channel);
    tree.addBranch(m_padTrigger_digits_relbcid);
    tree.addBranch(m_padTrigger_TP_R_id_init);
    tree.addBranch(m_padTrigger_TP_phi_id_init);
    tree.addBranch(m_padTrigger_TP_relbcid_init);
    tree.addBranch(m_padTrigger_TP_R_id);
    tree.addBranch(m_padTrigger_TP_phi_id);
    tree.addBranch(m_padTrigger_TP_relbcid);
    tree.addBranch(m_padTrigger_wheel);
    tree.addBranch(m_padTrigger_sector);
    tree.addBranch(m_padTrigger_hitmask);
    tree.addBranch(m_padTrigger_efficiency);

    return StatusCode::SUCCESS;
  }

  /*
   ***** Main algorithm tool
   */
  StatusCode PadEmulatorTool::emulate(const EventContext& ctx, Muon::NSW_PadTriggerDataContainer *out) const {

    SG::ReadCondHandle<MuonGM::MuonDetectorManager> detManager{m_detectorManagerKey, ctx};
    if(!detManager.isValid()){
      ATH_MSG_ERROR("Failed to retrieve the MuonDetectorManager conditions object");
      return StatusCode::FAILURE;
    }

    SG::ReadHandle<sTgcDigitContainer> sTGCdigits(m_sTGCDigitContainerKey, ctx);
    if(!sTGCdigits.isValid()) {
      ATH_MSG_ERROR("Failed to retrieve the sTGC Digit container");
      return StatusCode::FAILURE;
    }

    const NswDcsDbData* dcsData = nullptr;
    if(!m_isMC) {
      SG::ReadCondHandle<NswDcsDbData> dcsDataHandle{m_dcsKey, ctx};
      if(!dcsDataHandle.isValid()) {
        ATH_MSG_ERROR("Failed to retrieve DCS data while running on data");
        return StatusCode::FAILURE;
      }
      dcsData = dcsDataHandle.cptr();
    }

    const std::array<uint16_t, 8> bcWindow = {0, 1, 2, 3, 4, UINT16_MAX-1, UINT16_MAX-2, UINT16_MAX-3};
    for(const sTgcDigitCollection* digitCollection : *sTGCdigits) {

      std::vector<uint32_t> hits_sourceid{}, hits_pfeb{}, hits_padchan{}, hits_relbcid{};
      for(const sTgcDigit* digit : *digitCollection) {
        if(m_idHelperSvc->stgcIdHelper().channelType(digit->identify()) == sTgcIdHelper::sTgcChannelTypes::Pad) {
          if(std::find(bcWindow.begin(), bcWindow.end(), digit->bcTag()) != bcWindow.end()) {
            const Identifier Id = digit->identify();
            if(!m_isMC) {
              if(!dcsData->isConnectedChannel(Id) or !dcsData->isGood(ctx, Id) or !dcsData->isGoodHv(Id) or !dcsData->isGoodEltx(Id)) continue;
              bool disabled = false;
              if(!dcsData->isGoodTDaq(ctx,Id,disabled)) continue;
            }

            const int stEta = m_idHelperSvc->stationEta(Id);
            const int stPhi = m_idHelperSvc->stationPhi(Id);
            const int multilayer = m_idHelperSvc->stgcIdHelper().multilayer(Id);
            const int gasGap = m_idHelperSvc->stgcIdHelper().gasGap(Id);
            const int channel = m_idHelperSvc->stgcIdHelper().channel(Id);
            ATH_MSG_DEBUG("Digit: " << m_idHelperSvc->toString(Id) << " - BC: " << digit->bcTag());

            // Source ID retrieval: Pad Trigger logic needs sector number starting from 0
            int sector = m_idHelperSvc->sector(Id)-1;
            const char wheel = (stEta > 0) ? 'A' : 'C';
            auto sourceid = NSWL1::PAD::wheelSectorToSourceID(wheel, sector);
            hits_sourceid.emplace_back(sourceid);

            // pFEB retrieval
            auto pfeb = NSWL1::PAD::getpFEBAthena(gasGap, multilayer, stEta);
            hits_pfeb.emplace_back(pfeb);

            // padChannel retrieval: no check on the Type, as already done by selecting PAD digits only
            auto padchan = NSWL1::PAD::getPadchAthena(channel, pfeb, sector, gasGap);
            hits_padchan.emplace_back(padchan);

            // BC retrieval
            hits_relbcid.emplace_back(digit->bcTag());

            if(m_doNtuple) {
              m_padTrigger_digits_sector->push_back(sector);
              m_padTrigger_digits_eta->push_back(stEta);
              m_padTrigger_digits_phi->push_back(stPhi);
              m_padTrigger_digits_multilayer->push_back(multilayer);
              m_padTrigger_digits_gasgap->push_back(gasGap);
              m_padTrigger_digits_source_id->push_back(sourceid);
              m_padTrigger_digits_pfeb->push_back(pfeb);
              m_padTrigger_digits_pad_channel->push_back(padchan);
              m_padTrigger_digits_relbcid->push_back(digit->bcTag());
            }
          }
        }
      }
      ATH_MSG_DEBUG("Processed sTGC digit collection with size " << digitCollection->size() << ", finding a total of " << hits_sourceid.size() << " pad hits");

      for(const auto &rob : m_robIDs) {
        if(hits_sourceid.empty() or hits_pfeb.empty() or hits_padchan.empty() or hits_relbcid.empty()) continue;

        // Find robID
        std::vector<uint32_t> hits_indices{};
        for(size_t index{0}; index < hits_sourceid.size(); ++index) {
          if(hits_sourceid.at(index) == rob) hits_indices.emplace_back(index);
        }

        if(hits_indices.empty()) continue;
  
        // Preprocessing
        const auto& sourceid = hits_sourceid.at(hits_indices.front());
        const bool isLarge = (sourceid % 2 == 0);
        const bool isA = NSWL1::isA(sourceid);
        const char whl = isA ? 'A' : 'C';
        const auto sec = (sourceid & 0xf);  // last 4 bits of the sourceID are sector number (0-15)
        const auto& patterns = isLarge ? m_patterns_L : m_patterns_S;
        std::vector<uint32_t> masks(patterns.size() * NSWL1::PAD::PAD_TRIGGER_READOUT_NBC);

        // Loop over available hits
        for(const auto& index: hits_indices) {
          const auto& pfeb = hits_pfeb.at(index);
          const auto& chan = hits_padchan.at(index);
          const auto& bcid = m_ignoreBCIDs ? 0 : hits_relbcid.at(index);
          ATH_MSG_DEBUG("PadTrigger sector " << whl << sec+1 << std::dec << " -> Hit " << index << " pfeb " << pfeb << " padchan " << chan << " bcid " << bcid);

          /* What we are getting here:
           * producing a vector with size # of patterns * number of bunch crossings
           * so we have pattern A BCID 0, pattern B BCID 0,..., pattern A BCID 1, pattern B BCID 1
           * pattern A BCID 0 = 00000000 if no layers were hit, 00010101 if layers 0, 2, and 4 were hit
           * for each hit, find the patterns it belongs to
           * then for the corresponding bcid and the ones within stretch, mark the layer as hit
           */
          const auto layer = (pfeb % NSWL1::PAD::NPFEB_PER_RADIUS);
          for(size_t it{0}; it < patterns.size(); ++it) {
            if(patterns.at(it).getPfebs().at(layer) == pfeb and patterns.at(it).getPadChannels().at(layer) == chan) {
              for(uint32_t bc{bcid}; bc <= bcid + m_stretch; ++bc) {
                if(bc >= NSWL1::PAD::PAD_TRIGGER_READOUT_NBC) break;
                masks.at(it + bc*patterns.size()) |= (1 << layer);
              }
            }
          }
        }

        // Checking for problems/transitionregions/pads that are masked to 1
        std::string secstr = whl + std::to_string(sec+1);
        if(m_maskedPatterns.find(secstr) != m_maskedPatterns.end()) {
          for(const auto& [patind, tomask] : m_maskedPatterns.at(secstr)) {
            for(size_t bcid{0}; bcid < NSWL1::PAD::PAD_TRIGGER_READOUT_NBC; ++bcid) {
              masks.at(patind + bcid*patterns.size()) |= tomask;
            }
          }
        }

        // Check coincidences
        for(size_t relbcid{0}; relbcid < NSWL1::PAD::PAD_TRIGGER_READOUT_NBC; ++relbcid) {
          if(m_ignoreBCIDs and relbcid > 0) continue;

          bool isTrigger;
          std::vector<PadEmulatorTrigger> triggersBc{};
          for (size_t it{0}; it < patterns.size(); ++it) {
            const auto& pattern = patterns.at(it);
            const uint32_t hitmask = masks.at(it + relbcid*patterns.size());
            if(m_triggerLogic == "4over8") {
              isTrigger = trigger_4over8(hitmask & 0x01, hitmask & 0x02, hitmask & 0x04, hitmask & 0x08,
                                         hitmask & 0x10, hitmask & 0x20, hitmask & 0x40, hitmask & 0x80);
            }
            else if(m_triggerLogic == "3and1") {
              isTrigger = trigger_3and1(hitmask & 0x01, hitmask & 0x02, hitmask & 0x04, hitmask & 0x08,
                                        hitmask & 0x10, hitmask & 0x20, hitmask & 0x40, hitmask & 0x80);
            }
            else if(m_triggerLogic == "2and2") {
              isTrigger = trigger_2and2(hitmask & 0x01, hitmask & 0x02, hitmask & 0x04, hitmask & 0x08,
                                        hitmask & 0x10, hitmask & 0x20, hitmask & 0x40, hitmask & 0x80);
            }
            else if(m_triggerLogic == "specific4over8") {
              isTrigger = trigger_specific4over8(hitmask & 0x01, hitmask & 0x02, hitmask & 0x04, hitmask & 0x08,
                                                 hitmask & 0x10, hitmask & 0x20, hitmask & 0x40, hitmask & 0x80);
            }
            else if(m_triggerLogic == "5over8") {
              isTrigger = trigger_5over8(hitmask & 0x01, hitmask & 0x02, hitmask & 0x04, hitmask & 0x08,
                                         hitmask & 0x10, hitmask & 0x20, hitmask & 0x40, hitmask & 0x80);
            }
            else if(m_triggerLogic == "specific5over8") {
              isTrigger = trigger_specific5over8(hitmask & 0x01, hitmask & 0x02, hitmask & 0x04, hitmask & 0x08,
                                                 hitmask & 0x10, hitmask & 0x20, hitmask & 0x40, hitmask & 0x80);
            }
            else if(m_triggerLogic == "superspecific5over8") {
              isTrigger = trigger_superspecific5over8(hitmask & 0x01, hitmask & 0x02, hitmask & 0x04, hitmask & 0x08,
                                                      hitmask & 0x10, hitmask & 0x20, hitmask & 0x40, hitmask & 0x80);
            }
            else if(m_triggerLogic == "2x3over4") {
              isTrigger = trigger_2X_3over4(hitmask & 0x01, hitmask & 0x02, hitmask & 0x04, hitmask & 0x08,
                                            hitmask & 0x10, hitmask & 0x20, hitmask & 0x40, hitmask & 0x80);
            }
            else if(m_triggerLogic == "6over8") {
              isTrigger = trigger_6over8(hitmask & 0x01, hitmask & 0x02, hitmask & 0x04, hitmask & 0x08,
                                         hitmask & 0x10, hitmask & 0x20, hitmask & 0x40, hitmask & 0x80);
            }
            else if(m_triggerLogic == "8over8") {
              isTrigger = trigger_8over8(hitmask & 0x01, hitmask & 0x02, hitmask & 0x04, hitmask & 0x08,
                                         hitmask & 0x10, hitmask & 0x20, hitmask & 0x40, hitmask & 0x80);
            }
            else {
              ATH_MSG_ERROR("Unknown sTGC Pad Trigger logic!");
              return StatusCode::FAILURE;
            }
            if(not isTrigger) continue;

            // Save the output
            triggersBc.emplace_back(
                PadEmulatorTrigger(whl, sec, pattern.getBandid(), pattern.getPhiid(isA), relbcid, pattern, hitmask)
            );
          } // Closing loop on patterns
          if(m_doNtuple) {
            m_padTrigger_efficiency->push_back(triggersBc.size());
            for(const auto& trigger : triggersBc) {
              if(relbcid != trigger.getRelbcid()) continue;
              m_padTrigger_TP_R_id_init->push_back(trigger.getBandid());
              m_padTrigger_TP_phi_id_init->push_back(trigger.getPhiid());
              m_padTrigger_TP_relbcid_init->push_back(trigger.getRelbcid());
            }
          }

          // Filter output: eta bias
          triggersBc = (m_bandIDPriorityEncode) ? filterPriorityEncoder(triggersBc, isLarge) : filterLowBandIDs(triggersBc);
          if(m_doNtuple) m_padTrigger_efficiency->push_back(triggersBc.size());

          // Filter output: phi bias
          if(m_lastPhiOnly) triggersBc = filterLastPhiOnly(triggersBc);
          if(m_doNtuple) m_padTrigger_efficiency->push_back(triggersBc.size());

          // Filter output: duplicates of triggers with the same band and phi IDs in the same BC
          if(m_noDuplicates) triggersBc = filterDuplicates(triggersBc);
          if(m_doNtuple) m_padTrigger_efficiency->push_back(triggersBc.size());

          // Save the filtered output in the final container, for coincidence checks in the SL
          if(not triggersBc.empty()) {
            auto newCollection = new Muon::NSW_PadTriggerData(isA, sec, relbcid, 0);
            for(const auto& trigger : triggersBc) {
              if(relbcid != trigger.getRelbcid()) continue;
              newCollection->addTrigger(trigger.getBandid(), trigger.getPhiid(), 0);
              if(m_doNtuple) {
                m_padTrigger_TP_R_id->push_back(trigger.getBandid());
                m_padTrigger_TP_phi_id->push_back(trigger.getPhiid());
                m_padTrigger_TP_relbcid->push_back(trigger.getRelbcid());
                m_padTrigger_wheel->push_back(trigger.getWheel());
                m_padTrigger_sector->push_back(trigger.getSector());
                m_padTrigger_hitmask->push_back(trigger.getHitMask());
              }
            }
            if(out->addCollection(newCollection, out->numberOfCollections()).isFailure()) {
              ATH_MSG_ERROR("Failed while storing output in the Muon::NSW_PadTriggerDataContainer");
              return StatusCode::FAILURE;
            }
          }
        } // Closing loop on relative BCIDs
      } // Closing loop on ROBs
    } // Closing loop on digit collections
    return StatusCode::SUCCESS;
  }

  /*
   ***** Filter functions below:
   *** Retrieval of forbidden bandIDs
   *** Take into account eta bias
   *** Take into account phi bias
   *** Remove duplicates of triggers with the same band and phi IDs in the same BC
   */
  std::vector<uint32_t> PadEmulatorTool::getForbiddenBandIDs(const std::vector<PadEmulatorTrigger>& triggers,
                                                             const bool isLarge) const {
    std::set<uint32_t> forbiddens{};
    const auto& encoder = isLarge ? NSWL1::PAD::priorityEncoderL : NSWL1::PAD::priorityEncoderS;

    std::set<uint32_t> bands{};
    for(const auto& trigger : triggers) bands.emplace(trigger.getBandid());
    auto bandids = std::vector<uint32_t>(bands.cbegin(), bands.cend());

    std::sort(bandids.rbegin(), bandids.rend());
    for(const auto& bandid : bandids) {
      if(NSWL1::PAD::contains(forbiddens, bandid)) continue;

      for(uint32_t forb{bandid-1}; forb >= encoder(bandid); --forb) forbiddens.emplace(forb);
    }

    // bandid less than 6 is forbidden
    for(const auto& bandid : {0, 1, 2, 3, 4, 5}) forbiddens.emplace(bandid);

    return std::vector<uint32_t>(forbiddens.cbegin(), forbiddens.cend());
  }

  std::vector<PadEmulatorTrigger> PadEmulatorTool::filterPriorityEncoder(const std::vector<PadEmulatorTrigger>& input,
                                                                         const bool isLarge) const {
    if(input.empty()) return {};

    std::vector<PadEmulatorTrigger> output{};
    const auto& forbiddens = getForbiddenBandIDs(input, isLarge);
    for(const auto& trigger: input) {
      if(NSWL1::PAD::contains(forbiddens, trigger.getBandid())) continue;
      output.emplace_back(trigger);
    }
    return output;
  }

  std::vector<PadEmulatorTrigger> PadEmulatorTool::filterLowBandIDs(const std::vector<PadEmulatorTrigger>& input) const {
    if(input.empty()) return {};

    std::vector<PadEmulatorTrigger> output{};
    const std::set<uint32_t> forbiddens{0, 1, 2, 3, 4, 5};
    for(const auto& trigger : input) {
      if(NSWL1::PAD::contains(forbiddens, trigger.getBandid())) continue;
      output.emplace_back(trigger);
    }
    return output;
  }

  std::vector<PadEmulatorTrigger> PadEmulatorTool::filterLastPhiOnly(const std::vector<PadEmulatorTrigger>& input) const {
    if(input.empty()) return {};

    std::map<uint32_t, std::vector<uint32_t> > phisPerBand{};
    for(const auto& trigger : input) {
      phisPerBand.try_emplace(trigger.getBandid(), std::vector<uint32_t>());
      phisPerBand.at(trigger.getBandid()).emplace_back(trigger.getPhiid());
    }

    std::vector<PadEmulatorTrigger> output{};
    for(const auto& trigger : input) {
      if(trigger.getPhiid() != phisPerBand.at(trigger.getBandid()).back()) continue;
      output.emplace_back(trigger);
    }
    return output;
  }

  std::vector<PadEmulatorTrigger> PadEmulatorTool::filterDuplicates(const std::vector<PadEmulatorTrigger>& input) const {
    if(input.empty()) return {};

    std::vector<PadEmulatorTrigger> output{};
    for(const auto& trigger : input) {
      if(std::any_of(output.cbegin(), output.cend(), [&](const auto& cmp) {
        return trigger.getBandid() == cmp.getBandid() and trigger.getPhiid() == cmp.getPhiid();
        })) continue;
      output.emplace_back(trigger);
    }
    return output;
  }

  /*
   ***** Load patterns when initializing the algorithm tool
   */
  StatusCode PadEmulatorTool::loadPatterns(const std::string& pfile) {
    if(pfile.empty()) {
      ATH_MSG_ERROR("Empty filename string for pad pattern file. No pattern will be loaded");
      return StatusCode::FAILURE;
    }

    std::ifstream ifs(pfile);
    std::string line;
    if(ifs) {
      bool isLarge = false;
      uint32_t bandid = NSWL1::PAD::DUMMY_BANDID;
      while (not ifs.eof()) {
        std::getline(ifs, line);
        const auto words = CxxUtils::tokenize(line, NSWL1::SPACE);
        ATH_MSG_DEBUG("Got words of size " << words.size());

        // check for EOF line
        if(NSWL1::PAD::contains(line, NSWL1::PAD::PATTERN_END)) break;
        ATH_MSG_DEBUG("Not pattern end line ");

        // find the current bandID
        if(NSWL1::PAD::contains(line, NSWL1::PAD::PATTERN_TAG)) {
          isLarge = NSWL1::PAD::contains(line, NSWL1::PAD::LARGE);
          bandid = NSWL1::PAD::parseLineForBandid(line);
          continue;
        }
        ATH_MSG_DEBUG("Parsed line for band ID " << bandid << " for large sector " << isLarge);

        // skip uninteresting lines
        if(bandid == NSWL1::PAD::DUMMY_BANDID) continue;
        if(NSWL1::PAD::contains(line, NSWL1::SEMICOLON)) continue;
        if(NSWL1::PAD::contains(words, NSWL1::VHDLCOMMENT)) continue;
        if(std::any_of(std::begin(line), std::end(line), [](char c){ return std::isalpha(c); })) continue;
        if(words.size() < NSWL1::NLAYERS) continue;
        ATH_MSG_DEBUG("Skipped uninteresting lines");

        // clean up the line
        line = NSWL1::PAD::replace(line, ",", ", ");
        line = NSWL1::PAD::replace(line, "(", " ");
        line = NSWL1::PAD::replace(line, ")", " ");
        line = NSWL1::PAD::replace(line, "0-1", "-1");
        line = NSWL1::PAD::replace(line, " ", "");
        const auto vals = CxxUtils::tokenize(line, NSWL1::COMMA);
        if(vals.size() != NSWL1::PAD::PATTERNLEN) {
          throw std::runtime_error("Can't unpack " + line);
        }
        ATH_MSG_DEBUG("Cleaned up lines");

        // unpack the line
        const auto pfeb0    = static_cast<uint32_t>(std::stoul(vals.at(NSWL1::PAD::I_PFEB0)));
        const auto pfeb1    = static_cast<uint32_t>(std::stoul(vals.at(NSWL1::PAD::I_PFEB1)));
        const auto pfeb2    = static_cast<uint32_t>(std::stoul(vals.at(NSWL1::PAD::I_PFEB2)));
        const auto pfeb3    = static_cast<uint32_t>(std::stoul(vals.at(NSWL1::PAD::I_PFEB3)));
        const auto pfeb4    = static_cast<uint32_t>(std::stoul(vals.at(NSWL1::PAD::I_PFEB4)));
        const auto pfeb5    = static_cast<uint32_t>(std::stoul(vals.at(NSWL1::PAD::I_PFEB5)));
        const auto pfeb6    = static_cast<uint32_t>(std::stoul(vals.at(NSWL1::PAD::I_PFEB6)));
        const auto pfeb7    = static_cast<uint32_t>(std::stoul(vals.at(NSWL1::PAD::I_PFEB7)));

        const auto padchan0 = static_cast<uint32_t>(std::stoul(vals.at(NSWL1::PAD::I_PADCHAN0)));
        const auto padchan1 = static_cast<uint32_t>(std::stoul(vals.at(NSWL1::PAD::I_PADCHAN1)));
        const auto padchan2 = static_cast<uint32_t>(std::stoul(vals.at(NSWL1::PAD::I_PADCHAN2)));
        const auto padchan3 = static_cast<uint32_t>(std::stoul(vals.at(NSWL1::PAD::I_PADCHAN3)));
        const auto padchan4 = static_cast<uint32_t>(std::stoul(vals.at(NSWL1::PAD::I_PADCHAN4)));
        const auto padchan5 = static_cast<uint32_t>(std::stoul(vals.at(NSWL1::PAD::I_PADCHAN5)));
        const auto padchan6 = static_cast<uint32_t>(std::stoul(vals.at(NSWL1::PAD::I_PADCHAN6)));
        const auto padchan7 = static_cast<uint32_t>(std::stoul(vals.at(NSWL1::PAD::I_PADCHAN7)));
        const auto phiid    = static_cast<uint32_t>(std::stoul(vals.at(NSWL1::PAD::I_PHIID)));

        // Store the retrieved information
        const std::array<uint32_t,8> pfebs = {pfeb0, pfeb1, pfeb2, pfeb3, pfeb4, pfeb5, pfeb6, pfeb7};
        const std::array<uint32_t,8> padchans = {padchan0, padchan1, padchan2, padchan3, padchan4, padchan5, padchan6, padchan7};

        // Save information in the private member of the class
        if(isLarge) m_patterns_L.emplace_back( PadPattern(bandid, phiid, pfebs, padchans, isLarge) );
        else m_patterns_S.emplace_back( PadPattern(bandid, phiid, pfebs, padchans, isLarge) );
        ATH_MSG_DEBUG("Unpacked lines and info stored");
      }
    }
    else {
      ATH_MSG_ERROR("Unable to read pad pattern file. No pattern will be loaded");
      return StatusCode::FAILURE;
    }
    ifs.close();
    ATH_MSG_DEBUG("Successfully loaded list of patterns from " << pfile << " with size (L/S): " << m_patterns_L.size() << " / " << m_patterns_S.size());
    return StatusCode::SUCCESS;
  }

  /*
   ***** Mask patterns, according to known issues, when initializing the algorithm tool
   */
  StatusCode PadEmulatorTool::maskPatterns() {
    std::vector<char> sectorSizes = {'L', 'S'};
    for(const auto& secSize: sectorSizes) {
      const bool isLarge = (secSize == 'L');
      const std::vector<PadPattern> patterns = (isLarge) ? m_patterns_L : m_patterns_S;
      for(uint32_t patternitter{0}; patternitter < patterns.size(); ++patternitter) {
        const auto& pattern = patterns.at(patternitter);
        const auto pads = pattern.getPadChannels();
        uint32_t mask{0};
        bool ipfake = NSWL1::PAD::isDummyPad(pads.at(0));
        bool hofake = NSWL1::PAD::isDummyPad(pads.at(4));
        bool is4layer = (ipfake || hofake);

        std::map<std::string, uint32_t> problematicSectors{};
        if(is4layer) {
          int offset = (ipfake) ? 0 : 4;
          mask |= (1 << offset);
          if (m_triggerLogic != "specific4over8") mask |= (1 << (offset + 1));
          if (m_triggerLogic == "8over8" || m_triggerLogic == "2x3over4" || m_triggerLogic == "specific4over8") mask |= (1 << (offset + 2));
          if (m_triggerLogic == "8over8") mask |= (1 << (offset + 3));
          for (const auto& wheel: NSWL1::WHEELS) {
            for (const auto& sector: NSWL1::PAD::SECTORS) {
              if (isLarge != (sector%2 == 0)) continue;
              std::string secname = wheel + std::to_string(sector+1);
              problematicSectors.try_emplace(secname, mask);
            }
          }
        }

        if(problematicSectors.empty()) continue;
        for(const auto& sectoritr : problematicSectors) {
          const auto sector = sectoritr.first;
          m_maskedPatterns.try_emplace(sector, std::vector<std::tuple<uint32_t, uint32_t>>());
          m_maskedPatterns.at(sector).push_back(std::make_tuple(patternitter, sectoritr.second));
        }
      }
    }

    ATH_MSG_DEBUG("Found a total of " << m_maskedPatterns.size() << " patterns to be masked");
    return StatusCode::SUCCESS;
  }

  /*
   ***** Set ROB IDs when initializing the tool and to loop over them in the main algorithm loop
   */
  StatusCode PadEmulatorTool::createRobIDs() {
    for(const auto& wheel : NSWL1::WHEELS) {
      for(const auto& sector: NSWL1::PAD::SECTORS) {
        m_robIDs.emplace_back(((wheel == "A" ? NSWL1::PAD::MUON_STGC_ENDCAP_A_SIDE : NSWL1::PAD::MUON_STGC_ENDCAP_C_SIDE) << 16) | NSWL1::PAD::PAD_TRIGGER_ROB | sector);
      }
    }
    return StatusCode::SUCCESS;
  }
}
