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
#include "StoreGate/WriteDecorHandle.h"
#include "xAODTrigger/eFexTauRoIContainer.h"

namespace LVL1 {
// Doxygen class description below:
/** The eFEXtauBDTAlgo class calculates the tau BDT TOB variables
 */
    static const InterfaceID IID_IeFEXtauAlgoBase("LVL1::eFEXtauAlgoBase", 1, 0);

class eFEXtauAlgoBase : public AthAlgTool {

public:
    static const InterfaceID& interfaceID() { return IID_IeFEXtauAlgoBase; };
  /** Constructors */
  eFEXtauAlgoBase(const std::string &type, const std::string &name,
                  const IInterface *parent);

  /** Destructor */
  virtual ~eFEXtauAlgoBase();

  virtual StatusCode safetyTest();

  virtual void compute() {};
  virtual bool isCentralTowerSeed() const;
  virtual bool isBDT() const {return false;}
  virtual void
  setThresholds(const std::vector<unsigned int> & /*rHadThreshold*/,
                const std::vector<unsigned int> & /*bdtThreshold*/,
                unsigned int /*etThreshold*/,
                unsigned int /*etThresholdForRHad*/,
		unsigned int /*bdtMinEtThreshold*/, unsigned int /*etThresholdForRHadFrac*/) {};
  virtual void getRCore(std::vector<unsigned int> &rCoreVec) const;
  virtual unsigned int rCoreCore() const { return 0; }
  virtual unsigned int rCoreEnv() const { return 0; }
  virtual unsigned int rHadCore() const = 0;
  virtual unsigned int rHadEnv() const = 0;
  virtual float getRealRCore() const;
  virtual void getRHad(std::vector<unsigned int> &rHadVec) const;
  virtual float getRealRHad() const;
  virtual void getSums(unsigned int seed, bool UnD,
                       std::vector<unsigned int> &RcoreSums,
                       std::vector<unsigned int> &Remums);
  virtual unsigned int getBDTScore() const { return 0; }
  virtual unsigned int getBDTCondition() const { return 0; }
  virtual unsigned int getBDTHadFracCondition() const { return 0; }
  void setSCellEncoder(LVL1::eFEXtauTOB *tob) const;
    virtual void setup(int inputTable[3][3], int efex_id, int fpga_id, int central_eta) = 0;
    virtual std::unique_ptr<eFEXtauTOB> getTauTOB() const = 0;
    virtual unsigned int getEt() const = 0;
    virtual unsigned int getBitwiseEt() const = 0;
    virtual bool getUnD() const { return false; }
    virtual unsigned int getSeed() const = 0;
  protected:
   SG::ReadHandleKey<LVL1::eTowerContainer> m_eTowerContainerKey{
      this, "MyETowers", "eTowerContainer", "Input container for eTowers"};
  bool m_cellsSet = false;

  int m_eFexalgoTowerID[3][3]{};

  void buildLayers(int efex_id, int fpga_id, int central_eta);
  void setSCellPointers();
  void setSuperCells(eFEXtauTOB *tob, bool withSupercells);
  virtual void setSupercellSeed() {};
  virtual void setUnDAndOffPhi() {};




  unsigned int m_em0cells[3][3]{};
  unsigned int m_em1cells[12][3]{};
  unsigned int m_em2cells[12][3]{};
  unsigned int m_em3cells[3][3]{};
  unsigned int m_hadcells[3][3]{};
  unsigned int m_twrcells[3][3]{};

};

} // namespace LVL1

#endif
