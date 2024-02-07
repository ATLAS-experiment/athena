/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRTCONDITIONSALGS_TRTCONDPRINT_H
#define TRTCONDITIONSALGS_TRTCONDPRINT_H

/** @file TRTCondPrint.h
 * @brief Algorithm to dump TRT Conditions objects to text file
 * @author Peter Hansen <phansen@nbi.dk>
 **/

//
#include <string>
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ICondSvc.h"
#include "StoreGate/StoreGateSvc.h"
#include "TRT_ConditionsServices/ITRT_CalDbTool.h"
#include "StoreGate/DataHandle.h"
#include "InDetIdentifier/TRT_ID.h"
#include "TRT_ConditionsData/RtRelationMultChanContainer.h"
#include "TRT_ConditionsData/StrawT0MultChanContainer.h"
#include "AthenaKernel/IAthenaOutputStreamTool.h"
#include "GaudiKernel/EventIDRange.h"

/** @class TRTCondPrint
   Dump calibration constants from ConditionStore to text file
**/ 

class TRTCondPrint:public AthAlgorithm {
public:
  typedef TRTCond::RtRelationMultChanContainer RtRelationContainer ;
  typedef TRTCond::StrawT0MultChanContainer StrawT0Container ;


  /** constructor **/
  TRTCondPrint(const std::string& name, ISvcLocator* pSvcLocator);
  /** destructor **/
  ~TRTCondPrint(void);

  virtual StatusCode  initialize(void) override;    
  virtual StatusCode  execute(void) override;
  virtual StatusCode  finalize(void) override;

  /// create an TRTCond::ExpandedIdentifier from a TRTID identifier
  virtual TRTCond::ExpandedIdentifier trtcondid( const Identifier& id, int level = TRTCond::ExpandedIdentifier::STRAW) const;

  /// write calibration constants or errors to flat text file 
  virtual StatusCode writeCalibTextFile(std::ostream&) const;
  virtual StatusCode writeErrorTextFile(std::ostream&) const;


 private:

  ToolHandle<ITRT_CalDbTool> m_TRTCalDbTool;
  bool m_setup;                            //!< true at first event
  std::string m_par_caloutputfile;         //!< must be either nothing, caliboutput.txt or erroroutput.txt
  const TRT_ID* m_trtid;                   //!< trt id helper
  ServiceHandle<StoreGateSvc> m_detstore;

};
 
#endif // TRTCONDITIONSALGS_TRTCONDPRINT_H

