/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           IeFEXtauAlgo.h  -
//                              -------------------
//     begin                : 12 05 2020
//     email                :  nicholas.andrew.luongo@cern.ch
//  ***************************************************************************/

#ifndef IeFEXtauAlgo_H
#define IeFEXtauAlgo_H

#include "GaudiKernel/IAlgTool.h"
#include "L1CaloFEXSim/eFEXtauTOB.h"
#include "L1CaloFEXSim/eTowerContainer.h"

namespace LVL1 {
  
/*
Interface definition for eFEXtauAlgo
*/

  static const InterfaceID IID_IeFEXtauAlgo("LVL1::IeFEXtauAlgo", 1, 0);

  class IeFEXtauAlgo : virtual public IAlgTool {
  public:
    static const InterfaceID& interfaceID( ) ;

    virtual StatusCode safetyTest() = 0;
    virtual void setup(int inputTable[3][3], int efex_id, int fpga_id, int central_eta) = 0;
    virtual void compute() = 0;
    
    virtual bool isCentralTowerSeed() const = 0;
    virtual std::unique_ptr<eFEXtauTOB> getTauTOB() const = 0;
    virtual unsigned int rCoreCore() const = 0;
    virtual unsigned int rCoreEnv() const = 0;
    virtual void getRCore(std::vector<unsigned int> & rCoreVec) const = 0;
    virtual float getRealRCore() const = 0;
    virtual unsigned int rHadCore() const = 0;
    virtual unsigned int rHadEnv() const = 0;
    virtual void getRHad(std::vector<unsigned int> & rHadVec) const = 0;
    virtual float getRealRHad() const = 0;
    virtual unsigned int getEt() const = 0;
    virtual unsigned int getBitwiseEt() const = 0;
    virtual bool getUnD() const = 0;
    virtual unsigned int getSeed() const = 0;
    virtual void getSums(unsigned int seed, bool UnD, std::vector<unsigned int> & RcoreSums, 
                         std::vector<unsigned int> & RemSums) = 0;
    virtual unsigned int getBDTScore() const = 0;
    virtual unsigned int getBDTCondition() const = 0;
    virtual unsigned int getBDTHadFracCondition() const = 0;
    virtual bool isBDT() const = 0;

    virtual void setThresholds(const std::vector<unsigned int>& rHadThreshold,
		    const std::vector<unsigned int>& bdtThreshold,
		    unsigned int etThreshold,
		    unsigned int etThresholdForRHad,
		    unsigned int bdtMinEtThreshold) = 0;

  private:

  };

  inline const InterfaceID& LVL1::IeFEXtauAlgo::interfaceID()
  {
    return IID_IeFEXtauAlgo;
  }

} // end of namespace

#endif
