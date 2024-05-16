/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//
// LArRampFCalCorr.h - Algorithm to normalize FCal ramps and correct for 
//                     baseplane problems
//
// Author:  Alan Robinson <fbfree@interchange.ubc.ca>
// Created: August 28, 2007
//
//***************************************************************************

#ifndef LARRAMPFCALCORR_H
#define LARRAMPFCALCORR_H

#include "AthenaBaseComps/AthAlgorithm.h"

#include "LArRawConditions/LArRampComplete.h"

#include "CxxUtils/checker_macros.h"
class LArOnlineID;

class ATLAS_NOT_THREAD_SAFE LArRampFCalCorr : public AthAlgorithm
{
 public:
  LArRampFCalCorr(const std::string& name,ISvcLocator* pSvcLocator);
  ~LArRampFCalCorr();

  typedef LArRampComplete::ConstConditionsMapIterator LArRampIt;

  StatusCode initialize();
  StatusCode execute();
  StatusCode stop();
  StatusCode finalize(){return StatusCode::SUCCESS;}

 private: 

  static int toMod(int& slot);

  const LArOnlineID* m_onlineHelper{};

  float m_threshold{};
};

#endif
