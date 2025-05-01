/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           eFEXtauBDT.h  -
//                              -------------------
//     begin                : 15 09 2022
//     email                : david.reikher@cern.ch
//  ***************************************************************************/

#ifndef eFEXtauBDT_H
#define eFEXtauBDT_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "L1CaloFEXSim/eFEXBDT.h"
#include <vector>

class AthAlgTool;

namespace LVL1 {
// Doxygen class description below:
/** The eFEXtauBDT class calculates the tau TOB variables
 */

class eFEXtauBDT {

public:
  /** Constructors */
  eFEXtauBDT(AthAlgTool *log, std::string config_path);

  /** Destructor */
  virtual ~eFEXtauBDT();

  void next();

  void setPointerToSCell(int eta, int phi, int layer, unsigned int *sCellPtr);
  void setPointerToFracMultipliersParam(int index,
                                        unsigned int *fracMultipliers);
  void setPointerToBDTThresholdsParam(int index, unsigned int *bdtThresholds);
  void setPointerToETThresholdParam(unsigned int *etThreshold);
  void setPointerToMaxETParam(unsigned int *maxEtThreshold);
  void setPointerToMaxETParamFrac(unsigned int *maxEtThreshold);
  void setPointerToBDTMinETParam(unsigned int *bdtMinEtThreshold);

  void buildBDTVariables();
  void computeBDTScore();
  void computeETEstimate();
  void computeEMETEstimate();
  void computeHADETEstimate();
  void computeFracCondition();
  void computeBDTCondition();
  void computeTowers();
  void computeIsCentralTowerSeed();
  void debugPrintBDTVariables();
  void debugPrintTowers();
  void initBDTVars();
  void initETPointers();
  void initEMETPointers();
  void initHADETPointers();
  void initTowersPointers();
  unsigned int getETEstimate() const { return m_eTEstimate; }
  unsigned int getEMETEstimate() const { return m_EM_eTEstimate; }
  unsigned int getEMETEstimateOverflow() const {
    return m_EM_eTEstimateOverflow;
  }
  unsigned int getHADETEstimate() const { return m_HAD_eTEstimate; }
  unsigned int getHADETEstimateOverflow() const {
    return m_HAD_eTEstimateOverflow;
  }
  std::vector<unsigned int> &getBDTVars() { return m_bdtVars; }
  std::vector<unsigned int> &getTowers() { return m_towers; }
  std::vector<unsigned int> &getEMMultipliedByFracParams() {
    return m_emEtXMultiplier;
  }
  std::vector<unsigned int> &getEMMultipliedByFracParamsOverflow() {
    return m_emEtXMultiplierOverflow;
  }
  unsigned int getBDTScoreShifted() const { return m_bdtScoreShifted; }

  // ======= These are the important getters ======
  // These are the fields in the algorithm's output TOB structure in the
  // VHDL implementation of eFEXFirmware (TriggerObjectCore_tau type in VHDL)
  unsigned int getET() const;
  unsigned int getTOBETOverflow() const { return m_eTEstimateOverflow; }
  unsigned int getIsMax() const { return m_isSeeded; }
  unsigned int getBDTCondition() const { return m_bdtCondition; }
  unsigned int getFracCondition() const { return m_fracCondition; }
  unsigned int getBDTScore() const { return m_bdtScore; }
  // ================================================================================

  unsigned int multWithOverflow(unsigned int a, unsigned int b, bool &overflow,
                                int resultNBits);
  inline bool isOverflow(unsigned int number, int nBits);
  inline unsigned int BitLeftShift(unsigned int number, int by, int totalNBits);
  inline int flatTowerIndex(int eta, int phi);

private:
  void initPointers(const std::vector<std::vector<int>> &scells,
                    std::vector<unsigned int *> &ptr_list);
  unsigned int computeEstimate(std::vector<unsigned int *> &ptr_list,
                               bool &overflow, int resultNBits);

  unsigned int *superCellToPtr(int eta, int phi, int layer);

  unsigned int *m_em0cells[3][3]{};
  unsigned int *m_em1cells[12][3]{};
  unsigned int *m_em2cells[12][3]{};
  unsigned int *m_em3cells[3][3]{};
  unsigned int *m_hadcells[3][3]{};
  unsigned int *m_fracMultipliers[3]{};
  unsigned int *m_bdtThresholds[3]{};
  unsigned int *m_etThreshold{};
  unsigned int *m_maxEtThreshold{};
  unsigned int *m_maxEtThresholdFrac{};
  unsigned int *m_bdtMinEtThreshold{};
  unsigned int m_bdtScore = 0;
  unsigned int m_bdtScoreShifted = 0;
  unsigned int m_eTEstimate = 0;
  bool m_eTEstimateOverflow = false;
  unsigned int m_EM_eTEstimate = 0;
  bool m_EM_eTEstimateOverflow = 0;
  unsigned int m_HAD_eTEstimate = 0;
  bool m_HAD_eTEstimateOverflow = false;
  unsigned int m_fracCondition = 0;
  unsigned int m_bdtCondition = 0;
  bool m_isSeeded = false;
  unsigned int m_hadEstimateShifted = 0;
  std::vector<unsigned int> m_emEtXMultiplier;
  std::vector<unsigned int> m_emEtXMultiplierOverflow;
  std::vector<unsigned int> m_towers;

  // Final computed BDT variables
  std::vector<unsigned int> m_bdtVars;

  // BDT variables are sums of supercells. The pointers to the memory locations
  // containing the supercell ET values for each variable are stored in the
  // following vector. Index in the vector corresponds to index of the variable.
  std::vector<std::vector<unsigned int *>> m_bdtVarComputeSCellPointers;

  // List of pointers to supercells that participate in the sum to estimate the
  // ET
  std::vector<unsigned int *> m_eTComputeSCellPointers;
  std::vector<unsigned int *> m_EM_eTComputeSCellPointers;
  std::vector<unsigned int *> m_HAD_eTComputeSCellPointers;
  std::vector<std::vector<unsigned int *>> m_towersComputeSCellPointers;

  bool m_bdtVarsComputed = false;
  eFEXBDT m_bdt;
  AthAlgTool *m_log;
};

} // namespace LVL1

#endif
