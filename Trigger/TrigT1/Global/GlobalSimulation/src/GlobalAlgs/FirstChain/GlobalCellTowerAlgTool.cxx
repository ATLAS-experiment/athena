/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "GlobalCellTowerAlgTool.h"
#include "GlobalLArCell.h"


namespace GlobalSim {
  
  // Main constructor
  GlobalCellTowerAlgTool::GlobalCellTowerAlgTool(const std::string& type, const std::string& name, const IInterface* parent) :
    base_class(type, name, parent) {
  }


  // Initialize function running before first event
  StatusCode GlobalCellTowerAlgTool::initialize() {
       
    CHECK(m_gblLArCellContainerKey.initialize());
    CHECK(m_gblCellTowers.initialize());

    return StatusCode::SUCCESS;
  }


  // Main functional block running for each event
  StatusCode GlobalCellTowerAlgTool::run(const EventContext& ctx) const {

    ATH_MSG_DEBUG("Building cell towers");

    // Read in GlobalLArCellContainer
    auto h_gblLArCells = SG::makeHandle(m_gblLArCellContainerKey, ctx);
    CHECK(h_gblLArCells.isValid());
    const auto & gblLArCells = *h_gblLArCells;

    // Define tower array (98 eta bins x 64 phi bins)
    static constexpr int nEta{98};
    static constexpr int nPhi{64};

    auto pTowerEnergies = std::make_unique<std::array<std::array<float, nPhi>, nEta>>();
    auto & towerEnergies = *pTowerEnergies;
    for (const auto& cell : gblLArCells) {

        // Compute eta and phi indices (binning in steps of 0.1)
        int eta_index = static_cast<int>(std::floor(cell->eta() * 10)) + 49;
        int phi_index = static_cast<int>(std::floor(cell->phi() * 10)) + 32;

        // Ensure indices are within bounds
        if (eta_index < 0 || eta_index >= nEta || phi_index < 0 || phi_index >= nPhi) continue;

        // Add cell energy to the corresponding tower
        towerEnergies[eta_index][phi_index] += cell->getEnergy();
    }

    auto h_towerTOBs = SG::makeHandle(m_gblCellTowers, ctx);
    auto towers = std::make_unique<IOBitwise::CommonTOBContainer>();
    towers->reserve(nEta * nPhi);

    for (int etaBin = 0; etaBin < nEta; ++etaBin) {
        for (int phiBin = 0; phiBin < nPhi; ++phiBin) {

           if (towerEnergies[etaBin][phiBin] == 0) continue;
           int energyBits = std::clamp(static_cast<int>(towerEnergies[etaBin][phiBin]), 0, (1 << IOBitwise::CommonTOB::s_et_width) - 1);

           towers->emplace_back(std::make_shared<IOBitwise::CommonTOB>(std::bitset<IOBitwise::CommonTOB::s_et_width>(energyBits),
								       std::bitset<IOBitwise::CommonTOB::s_eta_width>(etaBin),
								       std::bitset<IOBitwise::CommonTOB::s_phi_width>(phiBin)));
        }
    }

    ATH_MSG_DEBUG("Built " << towers->size() << " cell towers and stored them as GenericTobs");

    CHECK(h_towerTOBs.record(std::move(towers))); 

    return StatusCode::SUCCESS;
  }

  // Overrides toString() function from base class, unused here
  std::string GlobalCellTowerAlgTool::toString() const {
      return {};
  }

} //namespace GlobalSim
