/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//*************************************************************************
//                          eFEXtauBDTAlgo  -  description
//                             --------------------
//    begin                 : 05 06 2020
//    email                 : david.reikher@cern.ch
//*************************************************************************

#include <iostream>

#include "L1CaloFEXSim/eFEXtauBDTAlgo.h"
#include "L1CaloFEXSim/eFEXtauTOB.h"
#include "L1CaloFEXSim/eTower.h"
#include "PathResolver/PathResolver.h"
#include <stdio.h> /* defines FILENAME_MAX */

// default constructor for persistency
LVL1::eFEXtauBDTAlgo::eFEXtauBDTAlgo(const std::string &type,
                                     const std::string &name,
                                     const IInterface *parent)
    : eFEXtauAlgoBase(type, name, parent) {}

/** Destructor */
LVL1::eFEXtauBDTAlgo::~eFEXtauBDTAlgo() {}

StatusCode LVL1::eFEXtauBDTAlgo::initialize() {
  ATH_CHECK(m_eTowerContainerKey.initialize());
  std::string configPath = PathResolver::find_file(
      m_bdtJsonConfigPath, "DATAPATH", PathResolver::RecursiveSearch);
  ATH_MSG_INFO("Using BDT config file " << configPath);
  if (configPath.size() == 0) {
    ATH_MSG_ERROR("Cannot locate BDT config file " << m_bdtJsonConfigPath);
    return StatusCode::FAILURE;
  }

  m_bdtAlgoImpl = std::make_unique<eFEXtauBDT>(this, configPath);

  try {
    setSCellPointers();
    setThresholdPointers();
    m_bdtAlgoImpl->initBDTVars();
    m_bdtAlgoImpl->initETPointers();
    m_bdtAlgoImpl->initEMETPointers();
    m_bdtAlgoImpl->initHADETPointers();
    m_bdtAlgoImpl->initTowersPointers();
  } catch (const std::domain_error &ex) {
    ATH_MSG_ERROR(ex.what());
    return StatusCode::FAILURE;
  }

  ATH_MSG_INFO("tau Algorithm version: BDT");
  return StatusCode::SUCCESS;
}

void LVL1::eFEXtauBDTAlgo::setup(int inputTable[3][3], int efex_id, int fpga_id,
                                 int central_eta) {

  std::copy(&inputTable[0][0], &inputTable[0][0] + 9, &m_eFexalgoTowerID[0][0]);

  buildLayers(efex_id, fpga_id, central_eta);
}

void LVL1::eFEXtauBDTAlgo::compute() { m_bdtAlgoImpl->next(); }

std::unique_ptr<LVL1::eFEXtauTOB> LVL1::eFEXtauBDTAlgo::getTauTOB() const {
  std::unique_ptr<eFEXtauTOB> tob = std::make_unique<eFEXtauTOB>();
  unsigned int et = getEt();
  tob->setEt(et);
  tob->setRcoreCore(rCoreCore());
  tob->setRcoreEnv(rCoreEnv());
  tob->setRhadCore(rHadCore());
  tob->setRhadEnv(rHadEnv());
  tob->setBitwiseEt(getBitwiseEt());
  tob->setIso(getRealRCore());
  tob->setSeedUnD(0);
  tob->setBDTScore(m_bdtAlgoImpl->getBDTScore());
  tob->setIsBDTAlgo(1);
  setSCellEncoder(tob.get());
  return tob;
}

void LVL1::eFEXtauBDTAlgo::setSCellPointers() {
  for (int phi = 0; phi < 3; phi++) {
    for (int eta = 0; eta < 3; eta++) {
      // Coarse layers
      m_bdtAlgoImpl->setPointerToSCell(eta, phi, 0, &m_em0cells[eta][phi]);
      m_bdtAlgoImpl->setPointerToSCell(eta, phi, 3, &m_em3cells[eta][phi]);
      m_bdtAlgoImpl->setPointerToSCell(eta, phi, 4, &m_hadcells[eta][phi]);
    }
    for (int eta = 0; eta < 12; eta++) {
      // Fine layers
      m_bdtAlgoImpl->setPointerToSCell(eta, phi, 1, &m_em1cells[eta][phi]);
      m_bdtAlgoImpl->setPointerToSCell(eta, phi, 2, &m_em2cells[eta][phi]);
    }
  }
}

void LVL1::eFEXtauBDTAlgo::setThresholdPointers() {
  for (int i = 0; i < 3; i++) {
    m_bdtAlgoImpl->setPointerToFracMultipliersParam(i,
                                                    &(m_hadFracMultipliers[i]));
  }

  for (int i = 0; i < 3; i++) {
    m_bdtAlgoImpl->setPointerToBDTThresholdsParam(i, &(m_bdtThresholds[i]));
  }

  m_bdtAlgoImpl->setPointerToMaxETParam(&m_maxEtThreshold);
  m_bdtAlgoImpl->setPointerToETThresholdParam(&m_etThreshold);
  m_bdtAlgoImpl->setPointerToBDTMinETParam(&m_bdtMinEtThreshold);
}

// Calculate reconstructed ET value
unsigned int LVL1::eFEXtauBDTAlgo::getEt() const {
  if (m_cellsSet == false) {
    ATH_MSG_DEBUG("Layers not built, cannot accurately calculate Et.");
  }

  return m_bdtAlgoImpl->getETEstimate();
}

unsigned int LVL1::eFEXtauBDTAlgo::rHadCore() const {
  if (m_cellsSet == false) {
    ATH_MSG_DEBUG("Layers not built, cannot calculate rHad core value");
  }

  return m_bdtAlgoImpl->getHADETEstimate();
}

unsigned int LVL1::eFEXtauBDTAlgo::rHadEnv() const {
  if (m_cellsSet == false) {
    ATH_MSG_DEBUG("Layers not built, cannot calculate rHad environment value");
  }

  return m_bdtAlgoImpl->getEMETEstimate();
}

// Return the bitwise value of the given Et
// See eFEXtauBaseAlgo for a first attempt at this
unsigned int LVL1::eFEXtauBDTAlgo::getBitwiseEt() const {
  if (m_cellsSet == false) {
    ATH_MSG_DEBUG("Layers not built, cannot accurately calculate Et.");
  }

  return m_bdtAlgoImpl->getET();
}

unsigned int LVL1::eFEXtauBDTAlgo::getBDTScore() const {
  return m_bdtAlgoImpl->getBDTScore();
}

unsigned int LVL1::eFEXtauBDTAlgo::getBDTCondition() const {
  return m_bdtAlgoImpl->getBDTCondition();
}

unsigned int LVL1::eFEXtauBDTAlgo::getBDTHadFracCondition() const {
  return m_bdtAlgoImpl->getFracCondition();
}

bool LVL1::eFEXtauBDTAlgo::isBDT() const { return true; }

void LVL1::eFEXtauBDTAlgo::setThresholds(
    const std::vector<unsigned int> &rHadThreshold,
    const std::vector<unsigned int> &bdtThreshold, unsigned int etThreshold,
    unsigned int maxEtThreshold, unsigned int bdtMinEtThreshold) {
  for (int i = 0; i < 3; i++) {
    m_hadFracMultipliers[i] = rHadThreshold[i];
    m_bdtThresholds[i] = bdtThreshold[i];
  }
  m_etThreshold = etThreshold;
  m_maxEtThreshold = maxEtThreshold;
  m_bdtMinEtThreshold = bdtMinEtThreshold;
}
