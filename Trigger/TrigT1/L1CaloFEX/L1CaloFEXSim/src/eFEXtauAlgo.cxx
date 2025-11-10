/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//*************************************************************************
//                          eFEXtauAlgo  -  description
//                             --------------------
//    begin                 : 06 05 2020
//    email                 : nicholas.andrew.luongo@cern.ch
//*************************************************************************

#include "eFEXtauAlgo.h"
#include "L1CaloFEXSim/eFEXtauTOB.h"
#include "L1CaloFEXSim/eTower.h"
#include <algorithm> //for std::copy
#include <vector>

// default constructor for persistency
LVL1::eFEXtauAlgo::eFEXtauAlgo(const std::string &type, const std::string &name,
                               const IInterface *parent)
    : eFEXtauAlgoBase(type, name, parent) {

}

/** Destructor */
LVL1::eFEXtauAlgo::~eFEXtauAlgo() {}

StatusCode LVL1::eFEXtauAlgo::initialize() {
  ATH_CHECK(m_eTowerContainerKey.initialize());

  ATH_MSG_INFO("tau Algorithm version: heuristic");
  return StatusCode::SUCCESS;
}

void LVL1::eFEXtauAlgo::setup(int inputTable[3][3], int efex_id, int fpga_id,
                              int central_eta) {

  std::copy(&inputTable[0][0], &inputTable[0][0] + 9, &m_eFexalgoTowerID[0][0]);

  buildLayers(efex_id, fpga_id, central_eta);
  setSupercellSeed();
  setUnDAndOffPhi();
}

std::unique_ptr<LVL1::eFEXtauTOB> LVL1::eFEXtauAlgo::getTauTOB() const {
  std::unique_ptr<eFEXtauTOB> tob = std::make_unique<eFEXtauTOB>();
  unsigned int et = getEt();
  tob->setEt(et);
  tob->setRcoreCore(rCoreCore());
  tob->setRcoreEnv(rCoreEnv());
  tob->setRhadCore(rHadCore());
  tob->setRhadEnv(rHadEnv());
  tob->setBitwiseEt(getBitwiseEt());
  tob->setIso(getRealRCore());
  tob->setSeedUnD(getUnD());
  tob->setBDTScore(0);
  tob->setIsBDTAlgo(0);
  return tob;
}

// Calculate reconstructed ET value
unsigned int LVL1::eFEXtauAlgo::getEt() const {
  if (m_cellsSet == false) {
    ATH_MSG_DEBUG("Layers not built, cannot accurately calculate Et.");
  }

  unsigned int out = 0;

  out += m_em0cells[0][1];
  out += m_em0cells[1][1];
  out += m_em0cells[2][1];
  out += m_em0cells[0][m_offPhi];
  out += m_em0cells[1][m_offPhi];
  out += m_em0cells[2][m_offPhi];

  out += m_em1cells[m_seed][1];
  out += m_em1cells[m_seed + 1][1];
  out += m_em1cells[m_seed + 2][1];
  out += m_em1cells[m_seed - 1][1];
  out += m_em1cells[m_seed - 2][1];
  out += m_em1cells[m_seed][m_offPhi];
  out += m_em1cells[m_seed + 1][m_offPhi];
  out += m_em1cells[m_seed + 2][m_offPhi];
  out += m_em1cells[m_seed - 1][m_offPhi];
  out += m_em1cells[m_seed - 2][m_offPhi];

  out += m_em2cells[m_seed][1];
  out += m_em2cells[m_seed + 1][1];
  out += m_em2cells[m_seed + 2][1];
  out += m_em2cells[m_seed - 1][1];
  out += m_em2cells[m_seed - 2][1];
  out += m_em2cells[m_seed][m_offPhi];
  out += m_em2cells[m_seed + 1][m_offPhi];
  out += m_em2cells[m_seed + 2][m_offPhi];
  out += m_em2cells[m_seed - 1][m_offPhi];
  out += m_em2cells[m_seed - 2][m_offPhi];

  out += m_em3cells[0][1];
  out += m_em3cells[1][1];
  out += m_em3cells[2][1];
  out += m_em3cells[0][m_offPhi];
  out += m_em3cells[1][m_offPhi];
  out += m_em3cells[2][m_offPhi];

  out += m_hadcells[0][1];
  out += m_hadcells[1][1];
  out += m_hadcells[2][1];
  out += m_hadcells[0][m_offPhi];
  out += m_hadcells[1][m_offPhi];
  out += m_hadcells[2][m_offPhi];

  // Overflow handling
  if (out > 0xffff)
    out = 0xffff;

  return out;
}

unsigned int LVL1::eFEXtauAlgo::rCoreCore() const {
  if (m_cellsSet == false) {
    ATH_MSG_DEBUG("Layers not built, cannot calculate rCore core value");
  }

  unsigned int out = 0;

  out += m_em2cells[m_seed][1];
  out += m_em2cells[m_seed + 1][1];
  out += m_em2cells[m_seed - 1][1];
  out += m_em2cells[m_seed][m_offPhi];
  out += m_em2cells[m_seed + 1][m_offPhi];
  out += m_em2cells[m_seed - 1][m_offPhi];

  // Overflow handling
  if (out > 0xffff)
    out = 0xffff;

  return out;
}

unsigned int LVL1::eFEXtauAlgo::rCoreEnv() const {
  if (m_cellsSet == false) {
    ATH_MSG_DEBUG("Layers not built, cannot calculate rCore environment value");
  }

  unsigned int out = 0;

  out += m_em2cells[m_seed + 2][1];
  out += m_em2cells[m_seed - 2][1];
  out += m_em2cells[m_seed + 3][1];
  out += m_em2cells[m_seed - 3][1];
  out += m_em2cells[m_seed + 4][1];
  out += m_em2cells[m_seed - 4][1];
  out += m_em2cells[m_seed + 2][m_offPhi];
  out += m_em2cells[m_seed - 2][m_offPhi];
  out += m_em2cells[m_seed + 3][m_offPhi];
  out += m_em2cells[m_seed - 3][m_offPhi];
  out += m_em2cells[m_seed + 4][m_offPhi];
  out += m_em2cells[m_seed - 4][m_offPhi];

  // Overflow handling
  if (out > 0xffff)
    out = 0xffff;

  return out;
}

unsigned int LVL1::eFEXtauAlgo::rHadCore() const {
  if (m_cellsSet == false) {
    ATH_MSG_DEBUG("Layers not built, cannot calculate rHad core value");
  }

  unsigned int out = 0;

  out += m_hadcells[0][1];
  out += m_hadcells[1][1];
  out += m_hadcells[2][1];
  out += m_hadcells[0][m_offPhi];
  out += m_hadcells[1][m_offPhi];
  out += m_hadcells[2][m_offPhi];

  // Overflow handling
  if (out > 0xffff)
    out = 0xffff;

  return out;
}

unsigned int LVL1::eFEXtauAlgo::rHadEnv() const {
  if (m_cellsSet == false) {
    ATH_MSG_DEBUG("Layers not built, cannot calculate rHad environment value");
  }

  unsigned int out = 0;

  out += m_em2cells[m_seed][1];
  out += m_em2cells[m_seed - 1][1];
  out += m_em2cells[m_seed + 1][1];
  out += m_em2cells[m_seed - 2][1];
  out += m_em2cells[m_seed + 2][1];
  out += m_em2cells[m_seed][m_offPhi];
  out += m_em2cells[m_seed - 1][m_offPhi];
  out += m_em2cells[m_seed + 1][m_offPhi];
  out += m_em2cells[m_seed - 2][m_offPhi];
  out += m_em2cells[m_seed + 2][m_offPhi];
  out += m_em1cells[m_seed][1];
  out += m_em1cells[m_seed - 1][1];
  out += m_em1cells[m_seed + 1][1];
  out += m_em1cells[m_seed][m_offPhi];
  out += m_em1cells[m_seed - 1][m_offPhi];
  out += m_em1cells[m_seed + 1][m_offPhi];

  // Overflow handling
  if (out > 0xffff)
    out = 0xffff;

  return out;
}

// Set the off phi value used to calculate ET and isolation
void LVL1::eFEXtauAlgo::setUnDAndOffPhi() {
  if (m_cellsSet == false) {
    ATH_MSG_DEBUG("Layers not built, cannot accurately set phi direction.");
  }

  unsigned int upwardEt = m_em2cells[m_seed][2];
  unsigned int downwardEt = m_em2cells[m_seed][0];

  if (m_AlgoVersion==3){
    upwardEt = m_em1cells[m_seed][2];
    downwardEt = m_em1cells[m_seed][0];
  }
  
  if (downwardEt > upwardEt) {
    m_offPhi = 0;
    m_und = false;
  } else {
    m_offPhi = 2;
    m_und = true;
  }
}

// Utility function to calculate and return jet discriminant sums for specified
// location Intended to allow xAOD TOBs to be decorated with this information
void LVL1::eFEXtauAlgo::getSums(unsigned int seed, bool UnD,
                                std::vector<unsigned int> &RcoreSums,
                                std::vector<unsigned int> &RemSums) {
  // Set seed parameters to supplied values
  m_und = UnD;
  m_seed = seed + 4; // In this function seed has range 4-7

  // Now just call the 2 discriminant calculation methods
  getRCore(RcoreSums);
  getRHad(RemSums);
}

// Find the supercell seed eta value, must be in central cell so in the range
// 4-7 inclusive
void LVL1::eFEXtauAlgo::setSupercellSeed() {
  unsigned int seed = 7;
  int max_et = 0;
  int cell_et = 0;
  for (unsigned int i = 7; i > 3; --i) {
    cell_et = m_em2cells[i][1];
    
    if (m_AlgoVersion==3){
      cell_et = m_em1cells[i][1];
    }
    if (cell_et > max_et) {
      seed = i;
      max_et = cell_et;
    }
  }
  m_seed = seed;
}

// Return the bitwise value of the given Et
// See eFEXtauBaseAlgo for a first attempt at this
unsigned int LVL1::eFEXtauAlgo::getBitwiseEt() const {
  unsigned int out = 0;
  return out;
}

bool LVL1::eFEXtauAlgo::getUnD() const { return m_und; }

unsigned int LVL1::eFEXtauAlgo::getSeed() const { return m_seed; }

void LVL1::eFEXtauAlgo::setAlgoVersion(unsigned int ver) {m_AlgoVersion = ver;}
