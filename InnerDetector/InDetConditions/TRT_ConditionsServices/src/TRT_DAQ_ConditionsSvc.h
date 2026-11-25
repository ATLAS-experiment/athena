/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRT_DAQ_CONDITIONSSVC_H
#define TRT_DAQ_CONDITIONSSVC_H

/*----------------------------------------------------
 *  @file TRT_DAQ_ConditionsSvc.h
 *  @Service Header for TRT_DAQ_ConditionsSvc
 *  @author Denver Whittington
 *///-------------------------------------------------

// Header Includes
#include "AthenaBaseComps/AthService.h"
#include "TRT_ConditionsServices/ITRT_DAQ_ConditionsSvc.h"
#include "TRT_ConditionsServices/ITRT_ConditionsSvc.h"
#include "GaudiKernel/ServiceHandle.h"
#include "StoreGate/StoreGateSvc.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "StoreGate/ReadCondHandleKey.h"

#include "Identifier/Identifier.h"
#include "InDetIdentifier/TRT_ID.h"

class CondAttrListCollection;

// Class definition for this AlgTool
class TRT_DAQ_ConditionsSvc : public extends<AthService,
                                             ITRT_ConditionsSvc,
                                             ITRT_DAQ_ConditionsSvc>
{

 public:

  using base_class::base_class;
  virtual ~TRT_DAQ_ConditionsSvc();

  virtual StatusCode initialize();
  virtual StatusCode finalize();

  // Return ROD identifier
  unsigned int RODid( const Identifier& );

  /// ConditionsSummaryStatus for a ROD ID
  /** Similar to ITRT_ConditionsSvc::condSummaryStatus,
   *  but specific to this service
   */
  InDet::TRT_CondFlag condSummaryStatus( unsigned int );

  /// @name Functions inherited from ITRT_ConditionsSvc
  //@{

  /// Evaluation for TRT_ConditionsSummarySvc
  /** Function inherited from ITRT_ConditionsSvc.
   *  Returns a InDet::TRT_CondFlag for the Identifier, as determined
   *  by the TRT_DAQ_ConditionSvc.
   */
  InDet::TRT_CondFlag condSummaryStatus( const Identifier& );

 private:

  ServiceHandle<StoreGateSvc> m_detStore{this, "DetectorStore", "DetectorStore"};

  // jobOptions properties
  StringProperty m_FolderName{this, "FolderName", "/TDAQ/Resources/ATLAS/TRT/Robins"}; // For run1 the name is /TDAQ/EnabledResources/ATLAS/TRT/Robins

  // Straw Helpers
  const TRT_ID* m_TRT_ID_Helper{};

  // Read Key
  SG::ReadCondHandleKey<CondAttrListCollection> m_RobinsReadKey{this,"RobinsReadKey","/TDAQ/Resources/ATLAS/TRT/Robins","Robins in-key"};

};

#endif // TRT_DAQ_CONDITIONSSVC_H
