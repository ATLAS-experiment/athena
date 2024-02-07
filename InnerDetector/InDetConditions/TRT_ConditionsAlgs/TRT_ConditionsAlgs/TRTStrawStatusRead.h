/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////
//TRTStrawStatusRead.h
// Algorithm to dump the straw status info in CondStore to text file 
//phansen@nbi.dk
////////////////////////////////////////////////////


#ifndef TRTSTRAWSTATUSREAD_H
#define TRTSTRAWSTATUSREAD_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/StoreGateSvc.h"
#include "TRT_ConditionsData/ExpandedIdentifier.h"
#include "TRT_ConditionsData/StrawStatusMultChanContainer.h"
#include "InDetIdentifier/TRT_ID.h"
#include "GaudiKernel/ToolHandle.h"
#include "TRT_ConditionsServices/ITRT_StrawStatusSummaryTool.h"

class TRTStrawStatusRead : public AthAlgorithm
{

 public:

  TRTStrawStatusRead( const std::string &name, ISvcLocator *pSvcLocator);
  virtual ~TRTStrawStatusRead()=default;
  typedef TRTCond::StrawStatusMultChanContainer StrawStatusContainer ;

  // Gaudi
  virtual StatusCode initialize( ) override;
  virtual StatusCode execute( ) override;
  virtual StatusCode finalize( ) override;


  virtual StatusCode writeToTextFile(const std::string& filename);

  virtual const StrawStatusContainer* getStrawStatusContainer() const;
  virtual const StrawStatusContainer* getStrawStatusPermanentContainer() const;
  virtual const StrawStatusContainer* getStrawStatusHTContainer() const;

 private:


  bool m_setup;                             //false before first event
  const TRT_ID* m_trtid;                    //!< trt id helper
  ToolHandle<ITRT_StrawStatusSummaryTool> m_status;
  //  ReadHandle  keys
  SG::ReadCondHandleKey<StrawStatusContainer> m_statReadKey{this,"StatReadKeyName","/TRT/Cond/Status","StrawStatus in-key"};
  SG::ReadCondHandleKey<StrawStatusContainer> m_permReadKey{this,"PermReadKeyName","/TRT/Cond/StatusPermanent","StrawStatusPermanent in-key"};
  SG::ReadCondHandleKey<StrawStatusContainer> m_statHTReadKey{this,"StatHTReadKeyName","/TRT/Cond/StatusHT","StrawStatusHT in-key"};

  // Which folder to print
  std::string m_printfolder;
};


#endif
