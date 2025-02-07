/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARMINBIAS2NTUPLE_H
#define LARMINBIAS2NTUPLE_H

#include "LArCalibTools/LArCond2NtupleBase.h"
#include "LArElecCalib/ILArMinBiasAverage.h"
#include "LArElecCalib/ILArMinBias.h"


class LArMinBias2Ntuple : public LArCond2NtupleBase
{
 public:
  LArMinBias2Ntuple(const std::string & name, ISvcLocator * pSvcLocator);
  ~LArMinBias2Ntuple();

  //standard algorithm methods
  virtual StatusCode initialize();
  virtual StatusCode stop();
  StatusCode finalize(){return StatusCode::SUCCESS;}
 private:

  SG::ReadCondHandleKey<ILArMinBias> m_contKey{this,"ContainerKey","LArMinBias"};
  SG::ReadCondHandleKey<ILArMinBiasAverage> m_contKeyAv{this,"ContainerKeyAv","LArPileupAverage"};
  bool m_isPileup;
};

#endif
