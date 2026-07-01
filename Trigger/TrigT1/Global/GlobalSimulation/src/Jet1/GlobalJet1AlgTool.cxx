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
  StatusCode GlobalJet1AlgTool::run(const EventContext& ctx) const {

    ATH_MSG_DEBUG("Building WTAConeJets");

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
    auto jets = std::make_unique<IOBitwise::CommonTOBContainer>();

    for(const auto& WTAJet: WTAJetList){
      int energyBits = std::clamp(static_cast<int>(WTAJet.pt()), 0, (1 << IOBitwise::CommonTOB::s_et_width) - 1);
      int etaBits = std::clamp(static_cast<int>(WTAJet.eta()), 0, (1 << IOBitwise::CommonTOB::s_eta_width) - 1);
      int phiBits = std::clamp(static_cast<int>(WTAJet.phi()), 0, (1 << IOBitwise::CommonTOB::s_phi_width) - 1);
      jets->emplace_back(new IOBitwise::CommonTOB(std::bitset<IOBitwise::CommonTOB::s_et_width>(energyBits),
						  std::bitset<IOBitwise::CommonTOB::s_eta_width>(etaBits),
						  std::bitset<IOBitwise::CommonTOB::s_phi_width>(phiBits)));
      // Print jet TOB
      ATH_MSG_DEBUG("Returning Jet: " << jets->back()->to_string());
    }

    ATH_MSG_DEBUG("Built " << jets->size() << " WTAConeJets and stored them as GenericTobs");

    CHECK(h_Jet1TOBs.record(std::move(jets))); 


    return StatusCode::SUCCESS;
  }

  // Overrides toString() function from base class, unused here
  std::string GlobalJet1AlgTool::toString() const {
      return {};
  }

} //namespace GlobalSim
