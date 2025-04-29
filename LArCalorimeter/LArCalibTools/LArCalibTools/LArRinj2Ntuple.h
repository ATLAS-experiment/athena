/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#ifndef LArRINJ2NTUPLE_H
#define LArRINJ2NTUPLE_H

#include "LArCalibTools/LArCond2NtupleBase.h"
#include "LArRawConditions/LArRinjComplete.h"


class LArRinj2Ntuple : public LArCond2NtupleBase
{
 public:
  LArRinj2Ntuple(const std::string & name, ISvcLocator * pSvcLocator);

  //standard algorithm methods
  virtual StatusCode stop();
  virtual StatusCode initialize();
  StatusCode finalize(){return StatusCode::SUCCESS;}
 private:

   SG::ReadCondHandleKey<LArRinjComplete> m_contKey{this, "ContainerKey",""};
};

#endif
