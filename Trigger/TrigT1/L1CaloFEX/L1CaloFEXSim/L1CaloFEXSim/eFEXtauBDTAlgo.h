/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           eFEXtauBDTAlgo.h  -
//                              -------------------
//     begin                : 14 07 2022
//     email                : david.reikher@cern.ch
//  ***************************************************************************/

#ifndef eFEXtauBDTAlgo_H
#define eFEXtauBDTAlgo_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaKernel/CLASS_DEF.h"
#include "L1CaloFEXSim/eFEXtauAlgoBase.h"
#include "L1CaloFEXSim/eFEXtauBDT.h"
#include "L1CaloFEXSim/eFEXtauTOB.h"
#include "L1CaloFEXSim/eTowerContainer.h"
//#include "L1CaloFEXToolInterfaces/IeFEXtauAlgo.h"

namespace LVL1 {
// Doxygen class description below:
/** The eFEXtauBDTAlgo class calculates the tau BDT TOB variables
 */

class eFEXtauBDTAlgo : public eFEXtauAlgoBase {

public:
  /** Constructors */
  eFEXtauBDTAlgo(const std::string &type, const std::string &name,
                 const IInterface *parent);

  /** standard Athena-Algorithm method */
  virtual StatusCode initialize() override;

  /** Destructor */
  virtual ~eFEXtauBDTAlgo();

  virtual void setup(int inputTable[3][3], int efex_id, int fpga_id,
                     int central_eta) override;
  virtual void compute() override;

  virtual std::unique_ptr<eFEXtauTOB> getTauTOB() const override;
  virtual unsigned int rHadCore() const override;
  virtual unsigned int rHadEnv() const override;
  virtual unsigned int getEt() const override;
  virtual unsigned int getBitwiseEt() const override;
  virtual unsigned int getBDTScore() const override;
  virtual unsigned int getBDTCondition() const override;
  virtual unsigned int getBDTHadFracCondition() const override;
  virtual bool isBDT() const override;
  virtual void setThresholds(const std::vector<unsigned int> &rHadThreshold,
                             const std::vector<unsigned int> &bdtThreshold,
                             unsigned int etThreshold,
                             unsigned int etThresholdForRHad,
			     unsigned int bdtMinEtThreshold) override;

  // Seed is hard-coded to 1 to reduce eta asymmetry 
  // which is stronger when it's hard-coded to 0 and 
  // also to avoid modifications in L1Topo
  virtual unsigned int getSeed() const override { return 1; }

protected:
private:
  StatusCode initBDTVars();

  void setSCellPointers();
  void setThresholdPointers();

  unsigned int m_hadFracMultipliers[3]{};
  unsigned int m_bdtThresholds[3]{};
  unsigned int m_etThreshold{};
  unsigned int m_maxEtThreshold{};
  unsigned int m_bdtMinEtThreshold{};

  unsigned int m_bdtScore{};

  // Final computed BDT variables
  std::vector<unsigned int> m_bdtVars;

  // BDT variables are sums of supercells. The pointers to the memory locations
  // containing the supercell ET values for each variable are stores in the
  // following vector. Index in the vector corresponds to index of the variable.
  std::vector<std::vector<unsigned int *>> m_bdtVarComputeSCellPointers;

  Gaudi::Property<std::string> m_bdtJsonConfigPath{
      this, "BDTJsonConfigPath", "",
      "Path to BDT json config file"};
  std::unique_ptr<eFEXtauBDT> m_bdtAlgoImpl;

  bool m_bdtVarsComputed = false;
};

} // namespace LVL1

// CLASS_DEF( LVL1::eFEXtauBDTAlgo , 140708609 , 1 )

#endif
