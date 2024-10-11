/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRT_CONDITIONSTESTSVC_H
#define TRT_CONDITIONSTESTSVC_H

/** @file TRT_ConditionsTestSvc.h
 *  @brief Service for example and test of TRT_ConditionsSummarySvc
 *  Inherits from ITRT_ConditionsSvc base class and returns InDet::TRT_GOOD.
 */

#include "TRT_ConditionsServices/ITRT_ConditionsTestSvc.h"
#include "TRT_ConditionsServices/ITRT_ConditionsSvc.h"
#include "AthenaBaseComps/AthService.h"

class Identifier;

class TRT_ConditionsTestSvc : public extends<AthService,
                                             ITRT_ConditionsSvc,
                                             ITRT_ConditionsTestSvc>
{

 public:

  TRT_ConditionsTestSvc( const std::string& name, ISvcLocator* pSvcLocator );
  virtual ~TRT_ConditionsTestSvc();

  virtual StatusCode initialize();
  virtual StatusCode finalize();

  /// @name Functions inherited from ITRT_ConditionsTestSvc
  //@{

  /// Test Method
  /** Function inherited from ITRT_ConditionsTestSvc.
   *  Simply prints "Hello World."
   */
  StatusCode test( const Identifier& );

  //@}

  /// @name Functions inherited from ITRT_ConditionsSvc
  //@{

  /// Evaluation for TRT_ConditionsSummarySvc
  /** Function inherited from ITRT_ConditionsSvc.
   *  Simply returns a InDet::TRT_GOOD.
   */
  InDet::TRT_CondFlag condSummaryStatus( const Identifier& );

  //@}

 private:

};

inline StatusCode TRT_ConditionsTestSvc::test( const Identifier& ) {
  ATH_MSG_INFO("Hello World! From TRT_ConditionsTestSvc.");
  return StatusCode::SUCCESS;
}

#endif // TRT_CONDITIONSTESTSVC_H
