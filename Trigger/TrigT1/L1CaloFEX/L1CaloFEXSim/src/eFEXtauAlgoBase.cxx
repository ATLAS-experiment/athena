/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//***********************************************************************************
//                           eFEXtauAlgoBase.cxx
//                          -------------------
//     begin                : 08 05 2023
//     email                : david.reikher@cern.ch,
//     nicholas.andrew.luongo@cern.ch
//  *********************************************************************************/

#include "L1CaloFEXSim/eFEXtauAlgoBase.h"

LVL1::eFEXtauAlgoBase::eFEXtauAlgoBase(const std::string &type,
                                       const std::string &name,
                                       const IInterface *parent)
    : AthAlgTool(type, name, parent) {
  declareInterface<eFEXtauAlgoBase>(this);
}

LVL1::eFEXtauAlgoBase::~eFEXtauAlgoBase() {}

StatusCode LVL1::eFEXtauAlgoBase::safetyTest() {

  SG::ReadHandle<eTowerContainer> eTowerContainer(
      m_eTowerContainerKey /*,ctx*/);
  if (!eTowerContainer.isValid()) {
    ATH_MSG_FATAL("Could not retrieve eTowerContainer "
                  << m_eTowerContainerKey.key());
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

// Build arrays holding cell ETs for each layer plus entire tower
void LVL1::eFEXtauAlgoBase::buildLayers(int efex_id, int fpga_id,
                                        int central_eta) {

  SG::ReadHandle<eTowerContainer> eTowerContainer(
      m_eTowerContainerKey /*,ctx*/);

  for (unsigned int ieta = 0; ieta < 3; ieta++) {
    for (unsigned int iphi = 0; iphi < 3; iphi++) {
      if (((efex_id % 3 == 0) && (fpga_id == 0) && (central_eta == 0) &&
           (ieta == 0)) ||
          ((efex_id % 3 == 2) && (fpga_id == 3) && (central_eta == 5) &&
           (ieta == 2))) {
        m_twrcells[ieta][iphi] = 0;
        m_em0cells[ieta][iphi] = 0;
        m_em3cells[ieta][iphi] = 0;
        m_hadcells[ieta][iphi] = 0;
        for (unsigned int i = 0; i < 4; i++) {
          m_em1cells[4 * ieta + i][iphi] = 0;
          m_em2cells[4 * ieta + i][iphi] = 0;
        }
      } else {
        const LVL1::eTower *tmpTower =
            eTowerContainer->findTower(m_eFexalgoTowerID[iphi][ieta]);
        m_twrcells[ieta][iphi] = tmpTower->getTotalET();
        m_em0cells[ieta][iphi] = tmpTower->getLayerTotalET(0);
        m_em3cells[ieta][iphi] = tmpTower->getLayerTotalET(3);
        m_hadcells[ieta][iphi] = tmpTower->getLayerTotalET(4);
        for (unsigned int i = 0; i < 4; i++) {
          m_em1cells[4 * ieta + i][iphi] = tmpTower->getET(1, i);
          m_em2cells[4 * ieta + i][iphi] = tmpTower->getET(2, i);
        }
      }
    }
  }
  m_cellsSet = true;
}

// Utility function to calculate and return jet discriminant sums for specified
// location Intended to allow xAOD TOBs to be decorated with this information
void LVL1::eFEXtauAlgoBase::getSums(unsigned int /*seed*/, bool /*UnD*/,
                                    std::vector<unsigned int> &RcoreSums,
                                    std::vector<unsigned int> &RemSums) {
  // Set seed parameters to supplied values

  // Now just call the 2 discriminant calculation methods
  getRCore(RcoreSums);
  getRHad(RemSums);
}

// Calculate the hadronic fraction isolation variable
void LVL1::eFEXtauAlgoBase::getRHad(std::vector<unsigned int> &rHadVec) const {
  unsigned int core = rHadCore();
  unsigned int env = rHadEnv();

  rHadVec.push_back(core);
  rHadVec.push_back(env);
}

// Calculate float isolation variable
float LVL1::eFEXtauAlgoBase::getRealRCore() const {
  unsigned int core = rCoreCore();
  unsigned int env = rCoreEnv();

  unsigned int num = core;
  unsigned int denom = core + env;

  float out = denom ? static_cast<float>(num) / static_cast<float>(denom) : 0;

  return out;
}

float LVL1::eFEXtauAlgoBase::getRealRHad() const {
  unsigned int core = rHadCore();
  unsigned int env = rHadEnv();

  unsigned int num = core;
  unsigned int denom = core + env;

  float out = denom ? static_cast<float>(num) / static_cast<float>(denom) : 0;

  return out;
}

void LVL1::eFEXtauAlgoBase::getRCore(
    std::vector<unsigned int> &rCoreVec) const {
  unsigned int core = rCoreCore();
  unsigned int env = rCoreEnv();

  rCoreVec.push_back(core);
  rCoreVec.push_back(env);
}

// Check if central tower qualifies as a seed tower for the tau algorithm
bool LVL1::eFEXtauAlgoBase::isCentralTowerSeed() const {
  // Need layer cell ET arrays to be built
  if (m_cellsSet == false) {
    ATH_MSG_DEBUG(
        "Layers not built, cannot accurately determine if a seed tower.");
  }

  bool out = true;

  // Get central tower ET
  unsigned int centralET = m_twrcells[1][1];

  // Loop over all cells and check that the central tower is a local maximum
  for (unsigned int beta = 0; beta < 3; beta++) {
    for (unsigned int bphi = 0; bphi < 3; bphi++) {
      // Don't need to compare central cell with itself
      if ((beta == 1) && (bphi == 1)) {
        continue;
      }

      // Cells to the up and right must have strictly lesser ET
      if (beta == 2 || (beta == 1 && bphi == 2)) {
        if (centralET <= m_twrcells[beta][bphi]) {
          out = false;
        }
      }
      // Cells down and to the left must have lesser or equal ET. If strictly
      // lesser would create zero TOB if two adjacent cells had equal energy
      else if (beta == 0 || (beta == 1 && bphi == 0)) {
        if (centralET < m_twrcells[beta][bphi]) {
          out = false;
        }
      }
    }
  }

  return out;
}