/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "GlobalJet1AlgTool.h"

#include "../IO/CommonTOB.h"

#include <bitset>


namespace GlobalSim {

  // Main constructor
  GlobalJet1AlgTool::GlobalJet1AlgTool(const std::string& type, const std::string& name, const IInterface* parent) :
    base_class(type, name, parent) {
  }


  // Initialize function running before first event
  StatusCode GlobalJet1AlgTool::initialize() {
       
    CHECK(m_gblCellTowers.initialize());
    CHECK(m_gblJet1JetsContainerKey.initialize());

    return StatusCode::SUCCESS;
  }


  // Main functional block running for each event
  StatusCode GlobalJet1AlgTool::run(const std::unique_ptr<IDataCollector>& dc,
				    const EventContext& ctx) const {

    ATH_MSG_DEBUG("Building WTAConeJets");
    if (dc){dc->collect(*this, "start");}


    // Read the GlobalCellTowers
    auto h_towerTOBs = SG::makeHandle(m_gblCellTowers, ctx);
    CHECK(h_towerTOBs.isValid());
    const auto & towers = *h_towerTOBs;
    const unsigned int inTopoTowersN = towers.size();
    ATH_MSG_DEBUG("Reading " << inTopoTowersN << " cell towers as GenericTobs");

    // Create bitwise towers
    std::vector<WTATrigObj> input_towers;
    for(unsigned int i = 0; i < inTopoTowersN; i++){
      // Has constituent tracking feature
      const IOBitwise::CommonTOB* tower = towers[i];
      WTATrigObj this_tower(tower->et_bits().to_ulong(), tower->eta_bits().to_ulong(), tower->phi_bits().to_ulong(), 0, i);
      input_towers.push_back(this_tower);
    }

    // Do the usual
    std::unique_ptr<WTAConeMaker> MyWTAConeMaker = std::make_unique<WTACone2PassMaker>();
    WTAParameters MyWTAParameters = WTAParameters(
      8, // const_et_cut = 2 GeV
      20, // seed_et_cut = 5 GeV
      4 // jet_dR = 4
    );
    MyWTAConeMaker->m_WTAConeMakerParameter = MyWTAParameters; // Pass the WTAConeParameters
    WTAConeParallelHelper wta_parallel_helper;
    wta_parallel_helper.SetBlockN(4);
    wta_parallel_helper.CreateBlocks(input_towers);
    wta_parallel_helper.RunParallelWTA(MyWTAConeMaker);
    wta_parallel_helper.CheckJetInCore();
    std::vector<WTAJet> WTAJetList = wta_parallel_helper.GetAllJets(); // Bitwise Jets

    auto h_Jet1TOBs = SG::makeHandle(m_gblJet1JetsContainerKey, ctx);
    auto jets = std::make_unique<IOBitwise::Jet1TOBContainer>();

    for(const auto& WTAJet: WTAJetList){
      int energyBits = std::clamp(static_cast<int>(WTAJet.pt()), 0, (1 << IOBitwise::CommonTOB::s_et_width) - 1);
      int etaBits = std::clamp(static_cast<int>(WTAJet.eta()), 0, (1 << IOBitwise::CommonTOB::s_eta_width) - 1);
      int phiBits = std::clamp(static_cast<int>(WTAJet.phi()), 0, (1 << IOBitwise::CommonTOB::s_phi_width) - 1);
      const IOBitwise::CommonTOB commonTOB{std::bitset<IOBitwise::CommonTOB::s_et_width>(energyBits),
					   std::bitset<IOBitwise::CommonTOB::s_eta_width>(etaBits),
					   std::bitset<IOBitwise::CommonTOB::s_phi_width>(phiBits)};

      // Ring energies and TOB counts, as JET1Alg fills them for its xAOD output
      WTA4JetERingInfo ering_info = WTAJet.GetERingInfo();
      int ringZeroPttBits = std::clamp(static_cast<int>(ering_info.ring0_Et), 0, (1 << IOBitwise::Jet1TOB::s_ring_zero_ptt_width) - 1);
      int ringOnePttBits = std::clamp(static_cast<int>(ering_info.ring1_Et), 0, (1 << IOBitwise::Jet1TOB::s_ring_one_ptt_width) - 1);
      int ringTwoPttBits = std::clamp(static_cast<int>(ering_info.ring2_Et), 0, (1 << IOBitwise::Jet1TOB::s_ring_two_ptt_width) - 1);
      int ringThreePttBits = std::clamp(static_cast<int>(ering_info.ring3_Et), 0, (1 << IOBitwise::Jet1TOB::s_ring_three_ptt_width) - 1);
      int ringFourPttBits = std::clamp(static_cast<int>(ering_info.ring4_Et), 0, (1 << IOBitwise::Jet1TOB::s_ring_four_ptt_width) - 1);
      int totalTobsBits = std::clamp(static_cast<int>(ering_info.total_TobN), 0, (1 << IOBitwise::Jet1TOB::s_total_tobs_width) - 1);
      int ringOneTobsBits = std::clamp(static_cast<int>(ering_info.ring1_TobN), 0, (1 << IOBitwise::Jet1TOB::s_ring_one_tobs_width) - 1);
      int ringTwoTobsBits = std::clamp(static_cast<int>(ering_info.ring2_TobN), 0, (1 << IOBitwise::Jet1TOB::s_ring_two_tobs_width) - 1);
      int ringThreeTobsBits = std::clamp(static_cast<int>(ering_info.ring3_TobN), 0, (1 << IOBitwise::Jet1TOB::s_ring_three_tobs_width) - 1);
      int ringFourTobsBits = std::clamp(static_cast<int>(ering_info.ring4_TobN), 0, (1 << IOBitwise::Jet1TOB::s_ring_four_tobs_width) - 1);

      jets->emplace_back(new IOBitwise::Jet1TOB(commonTOB,
						std::bitset<IOBitwise::Jet1TOB::s_ring_zero_ptt_width>(ringZeroPttBits),
						std::bitset<IOBitwise::Jet1TOB::s_ring_one_ptt_width>(ringOnePttBits),
						std::bitset<IOBitwise::Jet1TOB::s_ring_two_ptt_width>(ringTwoPttBits),
						std::bitset<IOBitwise::Jet1TOB::s_ring_three_ptt_width>(ringThreePttBits),
						std::bitset<IOBitwise::Jet1TOB::s_ring_four_ptt_width>(ringFourPttBits),
						std::bitset<IOBitwise::Jet1TOB::s_total_tobs_width>(totalTobsBits),
						std::bitset<IOBitwise::Jet1TOB::s_ring_one_tobs_width>(ringOneTobsBits),
						std::bitset<IOBitwise::Jet1TOB::s_ring_two_tobs_width>(ringTwoTobsBits),
						std::bitset<IOBitwise::Jet1TOB::s_ring_three_tobs_width>(ringThreeTobsBits),
						std::bitset<IOBitwise::Jet1TOB::s_ring_four_tobs_width>(ringFourTobsBits),
						// The WTA engine does not provide the flags yet, so they are
						// written as false, matching JET1Alg. Note et_overflow is
						// genuinely set in the reference data, so this is a
						// placeholder, not a final value.
						std::bitset<IOBitwise::Jet1TOB::s_truncate_width>(0),
						std::bitset<IOBitwise::Jet1TOB::s_next_tob_same_et_width>(0),
						std::bitset<IOBitwise::Jet1TOB::s_error_width>(0),
						std::bitset<IOBitwise::Jet1TOB::s_et_overflow_width>(0)));

      // Print jet TOB
      ATH_MSG_DEBUG("Returning Jet: " << jets->back()->to_string());
    }

    ATH_MSG_DEBUG("Built " << jets->size() << " WTAConeJets and stored them as GenericTobs");

    CHECK(h_Jet1TOBs.record(std::move(jets))); 

    if (dc){dc->collect(*this, "end");}

    return StatusCode::SUCCESS;
  }

  // Overrides toString() function from base class, unused here
  std::string GlobalJet1AlgTool::toString() const {
      return {};
  }

} //namespace GlobalSim
