/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ITRT_CONDITIONSTESTSVC_H
#define ITRT_CONDITIONSTESTSVC_H

/** @file TRT_ConditionsTestSvc.h
 *  @brief AlgTool for example and test of TRT_ConditionsSummarySvc.
 *  Simply returns InDet::TRT_GOOD.
 *  @author Denver Whittington
 */

#include "GaudiKernel/IService.h"

class Identifier;

class ITRT_ConditionsTestSvc : virtual public IService {

 public:
  DeclareInterfaceID(ITRT_ConditionsTestSvc,1,0);

  /// Test!
  virtual StatusCode test( const Identifier& ) = 0;

};

#endif // ITRT_CONDITIONSTESTSVC_H
