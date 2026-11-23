// This file's extension implies that it's C, but it's really -*- C++ -*-.

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARRECUTILS_LARFCALDEGTESTALG_H
#define LARRECUTILS_LARFCALDEGTESTALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "LArRecConditions/LArFcalDegFactors.h"


class LArFCalDegTestAlg
  : public AthReentrantAlgorithm
{
public:
  /**
   * @brief Constructor.
   */
  LArFCalDegTestAlg (const std::string& name,
                           ISvcLocator* pSvcLocator): AthReentrantAlgorithm (name, pSvcLocator){};


  /** 
   * @brief Standard Gaudi initialize method.
   */
  virtual StatusCode initialize() override;


  /** 
   * @brief Standard Gaudi execute method.
   */
  virtual StatusCode execute(const EventContext& ctx) const override;


private:
  SG::ReadCondHandleKey<LArFcalDegFactors> m_factorsKey { this
      , "degradationMap"
      , "FcalDegMap"
      , "SG Key for FCAL degradation coeffs. in CondStore" };
};




#endif // not LARRECUTILS_LARFCALDEGTESTALG_H
