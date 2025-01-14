/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//***********************************************************************************
//                           eFEXtauAlgoBase.h
//                          -------------------
//     begin                : 08 05 2023
//     email                : david.reikher@cern.ch,
//     nicholas.andrew.luongo@cern.ch
//  *********************************************************************************/

#ifndef eFEXtauAlgoBase_H
#define eFEXtauAlgoBase_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "L1CaloFEXSim/eFEXtauTOB.h"
#include "L1CaloFEXSim/eTowerContainer.h"
#include "L1CaloFEXToolInterfaces/IeFEXtauAlgo.h"
#include "StoreGate/WriteDecorHandle.h"
#include "xAODTrigger/eFexTauRoIContainer.h"

namespace LVL1 {
// Doxygen class description below:
/** The eFEXtauBDTAlgo class calculates the tau BDT TOB variables
 */

class eFEXtauAlgoBase : public AthAlgTool, virtual public IeFEXtauAlgo {

public:
  /** Constructors */
  eFEXtauAlgoBase(const std::string &type, const std::string &name,
                  const IInterface *parent);

  /** Destructor */
  virtual ~eFEXtauAlgoBase();

  virtual StatusCode safetyTest() override;

  virtual void compute() override{};
  virtual bool isCentralTowerSeed() const override;
  virtual bool isBDT() const override { return false; }
  virtual void
  setThresholds(const std::vector<unsigned int> & /*rHadThreshold*/,
                const std::vector<unsigned int> & /*bdtThreshold*/,
                unsigned int /*etThreshold*/,
                unsigned int /*etThresholdForRHad*/,
		unsigned int /*bdtMinEtThreshold*/) override{};
  virtual void getRCore(std::vector<unsigned int> &rCoreVec) const override;
  virtual unsigned int rCoreCore() const override { return 0; }
  virtual unsigned int rCoreEnv() const override { return 0; }
  virtual float getRealRCore() const override;
  virtual void getRHad(std::vector<unsigned int> &rHadVec) const override;
  virtual float getRealRHad() const override;
  virtual void getSums(unsigned int seed, bool UnD,
                       std::vector<unsigned int> &RcoreSums,
                       std::vector<unsigned int> &Remums) override;
  virtual unsigned int getBDTScore() const override { return 0; }
  virtual unsigned int getBDTCondition() const override { return 0; };
  virtual unsigned int getBDTHadFracCondition() const override { return 0; };
  void setSCellEncoder(LVL1::eFEXtauTOB *tob) const;

protected:
  SG::ReadHandleKey<LVL1::eTowerContainer> m_eTowerContainerKey{
      this, "MyETowers", "eTowerContainer", "Input container for eTowers"};
  bool m_cellsSet = false;

  int m_eFexalgoTowerID[3][3]{};

  void buildLayers(int efex_id, int fpga_id, int central_eta);
  void setSCellPointers();
  void setSuperCells(eFEXtauTOB *tob, bool withSupercells);
  virtual void setSupercellSeed() {}
  virtual void setUnDAndOffPhi() {}

  virtual bool getUnD() const override { return 0; }
  virtual unsigned int getSeed() const override { return 0; }

  unsigned int m_em0cells[3][3]{};
  unsigned int m_em1cells[12][3]{};
  unsigned int m_em2cells[12][3]{};
  unsigned int m_em3cells[3][3]{};
  unsigned int m_hadcells[3][3]{};
  unsigned int m_twrcells[3][3]{};

};

} // namespace LVL1

#endif
