/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @class TRTStrawStatusWrite.h
 * @author phansen@nbi.dk
 */

#ifndef TRT_CONDITIONSALGS_TRTSTRAWSTATUSWRITE_H
#define TRT_CONDITIONSALGS_TRTSTRAWSTATUSWRITE_H
#include <string>
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/DataHandle.h"
#include "StoreGate/StoreGateSvc.h"
#include "InDetIdentifier/TRT_ID.h"
#include "TRT_ConditionsData/ExpandedIdentifier.h"
#include "TRT_ConditionsData/StrawStatusMultChanContainer.h"
#include "AthenaKernel/IAthenaOutputStreamTool.h"
#include "TRT_ConditionsServices/ITRT_StrawStatusSummaryTool.h"

class TRTStrawStatusWrite : public AthAlgorithm
{

 public:

  TRTStrawStatusWrite( const std::string &name, ISvcLocator *pSvcLocator);
  virtual ~TRTStrawStatusWrite()=default;
  typedef TRTCond::StrawStatusMultChanContainer StrawStatusContainer ;

  // Gaudi
  virtual StatusCode initialize( ) override;
  virtual StatusCode execute( ) override;
  virtual StatusCode finalize( ) override;


  //special bits
  virtual void set_status_temp(StrawStatusContainer* ssc, Identifier offlineID, bool set);
  virtual void set_status_permanent(StrawStatusContainer* ssc, Identifier offlineID, bool set);

  //read text files
  virtual StatusCode readStatFromTextFile(const std::string& filename);
  virtual StatusCode readStatPermFromTextFile(const std::string& filename);
  virtual StatusCode readStatHTFromTextFile(const std::string& filename);

 private:


  ServiceHandle<StoreGateSvc> m_detStore;
  std::string m_par_strawstatuscontainerkey;
  std::string m_par_strawstatuspermanentcontainerkey;
  std::string m_par_strawstatusHTcontainerkey;
  std::string m_par_stattextfile;           //input text file
  std::string m_par_stattextfilepermanent;  //input text file: permanent
  std::string m_par_stattextfileHT;         //input text file: HT


  const TRT_ID* m_trtid;                    //TRT id helper
  ToolHandle<ITRT_StrawStatusSummaryTool> m_status;
};


#endif
