/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//*************************************************************************
//                          eFEXtauBDT  -  description
//                             --------------------
//    begin                 : 15 09 2022
//    email                 : david.reikher@gmail.com
//*************************************************************************

#include "L1CaloFEXSim/eFEXtauBDT.h"
#include <string>

#define ENERGY_WIDTH 16
#define PARAM_WIDTH 8

// default constructor for persistency
LVL1::eFEXtauBDT::eFEXtauBDT(AthAlgTool *log, std::string config_path)
    : m_bdt(config_path), m_log(log) {
  m_log->msg(MSG::DEBUG) << "Configured BDT with this file: " << config_path
                         << endmsg;
  m_bdtVars.resize(m_bdt.getVariables().size());
  m_towers.resize(m_bdt.getNTowers());
  int n_multipliers = sizeof(m_fracMultipliers) / sizeof(m_fracMultipliers[0]);
  m_emEtXMultiplier.resize(n_multipliers);
  m_emEtXMultiplierOverflow.resize(n_multipliers);
}

/** Destructor */
LVL1::eFEXtauBDT::~eFEXtauBDT() {}

void LVL1::eFEXtauBDT::setPointerToSCell(int eta, int phi, int layer,
                                         unsigned int *sCellPtr) {
  switch (layer) {
  case 0:
    m_em0cells[eta][phi] = sCellPtr;
    break;
  case 1:
    m_em1cells[eta][phi] = sCellPtr;
    break;
  case 2:
    m_em2cells[eta][phi] = sCellPtr;
    break;
  case 3:
    m_em3cells[eta][phi] = sCellPtr;
    break;
  case 4:
    m_hadcells[eta][phi] = sCellPtr;
    break;
  }
}

void LVL1::eFEXtauBDT::setPointerToFracMultipliersParam(
    int index, unsigned int *fracMultiplierPtr) {
  m_fracMultipliers[index] = fracMultiplierPtr;
}

void LVL1::eFEXtauBDT::setPointerToBDTThresholdsParam(
    int index, unsigned int *bdtThresholdPtr) {
  m_bdtThresholds[index] = bdtThresholdPtr;
}

void LVL1::eFEXtauBDT::setPointerToETThresholdParam(unsigned int *etThreshold) {
  m_etThreshold = etThreshold;
}

void LVL1::eFEXtauBDT::setPointerToMaxETParam(
    unsigned int *maxEtThreshold) {
  m_maxEtThreshold = maxEtThreshold;
}

void LVL1::eFEXtauBDT::setPointerToBDTMinETParam(
    unsigned int *bdtMinEtThreshold) {
  m_bdtMinEtThreshold = bdtMinEtThreshold;
}

unsigned int *LVL1::eFEXtauBDT::superCellToPtr(int eta, int phi, int layer) {
  unsigned int *ptr = 0;
  switch (layer) {
  case 0:
    ptr = m_em0cells[eta][phi];
    break;
  case 1:
    ptr = m_em1cells[eta][phi];
    break;
  case 2:
    ptr = m_em2cells[eta][phi];
    break;
  case 3:
    ptr = m_em3cells[eta][phi];
    break;
  case 4:
    ptr = m_hadcells[eta][phi];
    break;
  }
  return ptr;
}

void LVL1::eFEXtauBDT::initETPointers() {
  initPointers(m_bdt.getETSCells(), m_eTComputeSCellPointers);
}

void LVL1::eFEXtauBDT::initEMETPointers() {
  initPointers(m_bdt.getEMETSCells(), m_EM_eTComputeSCellPointers);
}

void LVL1::eFEXtauBDT::initHADETPointers() {
  initPointers(m_bdt.getHADETSCells(), m_HAD_eTComputeSCellPointers);
}

void LVL1::eFEXtauBDT::initPointers(const std::vector<std::vector<int>> &scells,
                                    std::vector<unsigned int *> &ptr_list) {
  m_log->msg(MSG::DEBUG) << "Will use sum of supercells: " << endmsg;
  for (const auto& scell : scells) {
    int eta = scell[0];
    int phi = scell[1];
    int layer = scell[2];
    m_log->msg(MSG::DEBUG) << "\teta=" << eta << "\tphi=" << phi
                           << "\tlayer=" << layer << endmsg;
    unsigned int *ptr = superCellToPtr(eta, phi, layer);
    if (ptr == 0) {
      m_log->msg(MSG::DEBUG)
          << "Could not convert eta=" << eta << " phi=" << phi
          << " layer=" << layer
          << " to a pointer to supercell. Are they within range?" << endmsg;
      throw std::domain_error(
          std::string("Could not convert eta=") + std::to_string(eta) +
          " phi=" + std::to_string(phi) + " layer=" + std::to_string(layer) +
          " to a pointer to supercell. Are they within range?");
    }
    ptr_list.push_back(ptr);
  }
}

void LVL1::eFEXtauBDT::initTowersPointers() {
  m_towersComputeSCellPointers.resize(m_bdt.getNTowers());
  for (size_t i = 0; i < m_towers.size(); i++) {
    m_log->msg(MSG::DEBUG) << "Tower " << i << endmsg;
    initPointers(m_bdt.getTowerSCells(i), m_towersComputeSCellPointers[i]);
  }
}

void LVL1::eFEXtauBDT::initBDTVars() {
  for (size_t i = 0; i < m_bdt.getVariables().size(); i++) {
    BDTVariable var = m_bdt.getVariables()[i];

    m_log->msg(MSG::DEBUG) << i << " is " << var.m_name << ", sum of supercells"
                           << endmsg;
    std::vector<unsigned int *> pointersToSCells;
    for (size_t j = 0; j < var.m_scells.size(); j++) {
      int eta = var.m_scells[j][0];
      int phi = var.m_scells[j][1];
      int layer = var.m_scells[j][2];
      m_log->msg(MSG::DEBUG) << "\teta=" << eta << "\tphi=" << phi
                             << "\tlayer=" << layer << endmsg;
      unsigned int *ptr = superCellToPtr(eta, phi, layer);
      if (ptr == 0) {
        m_log->msg(MSG::DEBUG)
            << "Could not convert eta=" << eta << " phi=" << phi
            << " layer=" << layer
            << " to a pointer to supercell. Are they within range?" << endmsg;
        throw std::domain_error(
            std::string("Could not convert eta=") + std::to_string(eta) +
            " phi=" + std::to_string(phi) + " layer=" + std::to_string(layer) +
            " to a pointer to supercell. Are they within range?");
      }
      pointersToSCells.push_back(ptr);
    }

    m_bdtVarComputeSCellPointers.push_back(pointersToSCells);
  }
}

void LVL1::eFEXtauBDT::next() {
  buildBDTVariables();
  computeTowers();
  computeETEstimate();
  computeHADETEstimate();
  computeEMETEstimate();
  computeBDTCondition();
  computeFracCondition();
  computeIsCentralTowerSeed();
}

void LVL1::eFEXtauBDT::debugPrintBDTVariables() {
  std::string bdtVariables = "";
  for (size_t i = 0; i < m_bdtVars.size(); i++) {
    bdtVariables += std::to_string(m_bdtVars[i]) + " ";
  }

  m_log->msg(MSG::DEBUG) << "BDT Variables: " << bdtVariables << endmsg;
}

// Build BDT Variables
void LVL1::eFEXtauBDT::buildBDTVariables() {
  for (size_t i = 0; i < m_bdtVarComputeSCellPointers.size(); i++) {
    bool overflow;
    m_bdtVars[i] = computeEstimate(m_bdtVarComputeSCellPointers[i], overflow,
                                   ENERGY_WIDTH);
    if (overflow) {
      m_bdtVars[i] = (1 << ENERGY_WIDTH) - 1;
    }
  }
  debugPrintBDTVariables();
  m_bdtVarsComputed = true;
}

void LVL1::eFEXtauBDT::computeBDTScore() {
  // Need BDT variables to be computed
  if (m_bdtVarsComputed == false) {
    m_log->msg(MSG::DEBUG)
        << "BDT Variables not computed. BDT score will be garbage." << endmsg;
  }

  m_bdtScore = m_bdt.getBDT().decision_function(m_bdtVars)[0];
  m_log->msg(MSG::DEBUG) << "BDT Score: " << m_bdtScore << endmsg;
}

void LVL1::eFEXtauBDT::computeETEstimate() {
  m_eTEstimate = computeEstimate(m_eTComputeSCellPointers, m_eTEstimateOverflow,
                                 ENERGY_WIDTH);
  m_log->msg(MSG::DEBUG) << "ET Estimate: " << m_eTEstimate << endmsg;
}

void LVL1::eFEXtauBDT::computeEMETEstimate() {
  m_EM_eTEstimate = computeEstimate(m_EM_eTComputeSCellPointers,
                                    m_EM_eTEstimateOverflow, ENERGY_WIDTH);
  m_log->msg(MSG::DEBUG) << "EM ET Estimate: " << m_EM_eTEstimate << endmsg;
}

void LVL1::eFEXtauBDT::computeHADETEstimate() {
  m_HAD_eTEstimate = computeEstimate(m_HAD_eTComputeSCellPointers,
                                     m_HAD_eTEstimateOverflow, ENERGY_WIDTH);
  m_log->msg(MSG::DEBUG) << "HAD ET Estimate: " << m_HAD_eTEstimate << endmsg;
}

void LVL1::eFEXtauBDT::debugPrintTowers() {
  m_log->msg(MSG::DEBUG) << "Towers Estimate: " << endmsg;
  for (int eta = 0; eta < 3; eta++) {
    for (int phi = 0; phi < 3; phi++) {
      int flatIndex = flatTowerIndex(eta, phi);
      m_log->msg(MSG::DEBUG)
          << "Tower " << flatIndex << " ET (eta=" << eta << ", phi=" << phi
          << "): " << m_towers[flatIndex] << endmsg;
    }
  }
}

int LVL1::eFEXtauBDT::flatTowerIndex(int eta, int phi) { return 3 * phi + eta; }

void LVL1::eFEXtauBDT::computeTowers() {
  for (size_t i = 0; i < m_towers.size(); i++) {
    bool overflow;
    m_towers[i] = computeEstimate(m_towersComputeSCellPointers[i], overflow,
                                  ENERGY_WIDTH);
    if (overflow) {
      m_towers[i] = (1 << ENERGY_WIDTH) - 1;
    }
  }

  debugPrintTowers();
}

bool LVL1::eFEXtauBDT::isOverflow(unsigned int number, int nBits) {
  if ((number >> nBits) != 0) {
    return true;
  }
  return false;
}

unsigned int
LVL1::eFEXtauBDT::computeEstimate(std::vector<unsigned int *> &ptr_list,
                                  bool &overflow, int resultNBits) {
  unsigned int estimate = 0;
  overflow = false;
  for (unsigned int *it : ptr_list) {
    estimate += *it;
    if (isOverflow(estimate, resultNBits)) {
      overflow = true;
    }
  }
  return estimate;
}

unsigned int LVL1::eFEXtauBDT::multWithOverflow(unsigned int a, unsigned int b,
                                                bool &overflow,
                                                int resultNBits) {
  overflow = false;
  if (b == 0) {
    return 0;
  }
  unsigned int result = a * b;
  if (a != result / b) {
    // This shouldn't happen (a and b are in reality 16 bit numbers), but just
    // in case.
    overflow = true;
  }

  overflow = isOverflow(result, resultNBits);

  return result;
}

unsigned int LVL1::eFEXtauBDT::BitLeftShift(unsigned int number, int by,
                                            int totalNBits) {
  if ((number >> (totalNBits - by)) != 0) {
    return (1 << totalNBits) - 1;
  }
  return number << by;
}

void LVL1::eFEXtauBDT::computeFracCondition() {
  int n_multipliers = sizeof(m_fracMultipliers) / sizeof(m_fracMultipliers[0]);

  if ((m_eTEstimate >= *m_maxEtThreshold) or m_eTEstimateOverflow or
      m_HAD_eTEstimateOverflow) {

    m_fracCondition = (1 << (n_multipliers - 1)) - 1;
    return;
  }

  if (m_EM_eTEstimateOverflow) {
    m_fracCondition = 0;
    return;
  }

  m_hadEstimateShifted = BitLeftShift(m_HAD_eTEstimate, 3, ENERGY_WIDTH);
  int i = 0;
  for (; i < n_multipliers; i++) {

    bool overflow;
    m_emEtXMultiplier[i] = multWithOverflow(
        *(m_fracMultipliers[i]), m_EM_eTEstimate, overflow, ENERGY_WIDTH);
    m_emEtXMultiplierOverflow[i] = (int)overflow;

    if ((m_hadEstimateShifted < m_emEtXMultiplier[i]) or
        m_emEtXMultiplierOverflow[i]) {
      break;
    }
  }
  m_fracCondition = i;
}

void LVL1::eFEXtauBDT::computeBDTCondition() {
  computeBDTScore();
  int n_thresholds = sizeof(m_bdtThresholds) / sizeof(m_bdtThresholds[0]);

  int toShiftRight = m_bdt.getScorePrecision() - PARAM_WIDTH;
  // Only compare the MSB bits of the BDT score to the thresholds provided in
  // the parameters
  m_bdtScoreShifted = (m_bdtScore >> toShiftRight);

  if ((m_eTEstimate >= *m_maxEtThreshold) or m_eTEstimateOverflow or
      m_eTEstimate < *m_bdtMinEtThreshold) {

    m_bdtCondition = (1 << (n_thresholds - 1)) - 1;
    return;
  }

  int i = 0;
  for (; i < n_thresholds; i++) {
    if (m_bdtScoreShifted < *(m_bdtThresholds[i])) {
      break;
    }
  }
  m_bdtCondition = i;
}

// Check if central tower qualifies as a seed tower for the tau algorithm
// Bitwise computation (as opposed to implementation in eFEXtauBDTAlgo.cxx)
void LVL1::eFEXtauBDT::computeIsCentralTowerSeed() {
  m_isSeeded = true;

  // Get central tower ET
  unsigned int centralET = m_towers[4];

  // Loop over all cells and check that the central tower is a local maximum
  for (unsigned int beta = 0; beta < 3; beta++) {
    for (unsigned int bphi = 0; bphi < 3; bphi++) {
      int flatIndex = flatTowerIndex(beta, bphi);
      // Don't need to compare central cell with itself
      if ((beta == 1) && (bphi == 1)) {
        continue;
      }

      // Cells to the up and right must have strictly lesser ET
      if (beta == 2 || (beta == 1 && bphi == 2)) {
        if (centralET <= m_towers[flatIndex]) {
          m_isSeeded = false;
        }
      }
      // Cells down and to the left must have lesser or equal ET. If strictly
      // lesser would create zero TOB if two adjacent cells had equal energy
      else if (beta == 0 || (beta == 1 && bphi == 0)) {
        if (centralET < m_towers[flatIndex]) {
          m_isSeeded = false;
        }
      }
    }
  }

  if (m_eTEstimate < *m_etThreshold) {
    m_isSeeded = false;
  }

  m_log->msg(MSG::DEBUG) << "Seeded: " << (int)m_isSeeded << endmsg;
}

unsigned int LVL1::eFEXtauBDT::getET() const {
  if (m_eTEstimateOverflow) {
    return (1 << ENERGY_WIDTH) - 1;
  }
  if (m_eTEstimate < *m_etThreshold) {
    return 0;
  }
  return m_eTEstimate;
}
