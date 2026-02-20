/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARROD_LARFEBERRORSUMMARYMAKER
#define LARROD_LARFEBERRORSUMMARYMAKER


/********************************************************************

NAME:     LArFebSummaryMaker
          Algorithm that makes LArFebSummary 

********************************************************************/

#include <AthenaBaseComps/AthReentrantAlgorithm.h>
#include <CxxUtils/checker_macros.h>
#include <GaudiKernel/ServiceHandle.h>
#include <GaudiKernel/ToolHandle.h>

#include "LArRawEvent/LArFebHeaderContainer.h"
#include "LArRawEvent/LArFebErrorSummary.h"
#include "LArRecConditions/LArBadChannelCont.h"

#include "StoreGate/WriteDecorHandleKey.h"
#include "xAODEventInfo/EventInfo.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"

#include "CxxUtils/checker_macros.h"

#include <array>
#include <set>

class LArOnlineID; 

class LArFebErrorSummaryMaker : public AthReentrantAlgorithm
{
 public:

  using AthReentrantAlgorithm::AthReentrantAlgorithm; 
  virtual ~LArFebErrorSummaryMaker() = default;

  virtual StatusCode initialize() override final;
  virtual StatusCode execute(const EventContext& ctx) const override final;
  virtual StatusCode finalize() override final;

 private:

  //Counters:
  mutable std::atomic<int> m_missingFebsWarns{0}; //counter for missing FEB warnings
  //Lock used when keeping track of errors (rare)
  mutable std::mutex m_mtx; //mutex to guards write-ops on the following objects:
  mutable std::array<unsigned, LArFebErrorSummary::N_LArFebErrorType> m_errors ATLAS_THREAD_SAFE; //error types accumulator
  mutable std::map<unsigned,unsigned> m_errsPerFeb ATLAS_THREAD_SAFE;


  //The following variables are set in initialize:
  std::set<unsigned int> m_all_febs ; 
  bool m_isHec=false;
  bool m_isFcal=false;
  bool m_isEmb=false;
  bool m_isEmec=false;
  bool m_isEmPS=false;
  bool m_isAside=false;
  bool m_isCside=false;

  const LArOnlineID* m_onlineHelper=nullptr;

  // properties:
  Gaudi::Property<int> m_warnLimit{ this, "warnLimit", 10, "Limit the number of warning messages for missing input" };
  Gaudi::Property<bool> m_checkAllFeb{ this, "CheckAllFEB", true, "Check all FEBS ?" };
  Gaudi::Property<std::string> m_partition{ this, "PartitionId", "", "Should contain DAQ partition (+ eventually the EventBuilder)" };
  Gaudi::Property<std::set<unsigned int> > m_knownEvtId{ this, "MaskFebEvtId", {}, "ignore these FEBs for EvtId" };
  Gaudi::Property<std::set<unsigned int> > m_knownSCACStatus{ this, "MaskFebScacStatus", {}, "ignore these FEBs for ScacStatus" };
  Gaudi::Property<std::set<unsigned int> > m_knownZeroSample{ this, "MaskFebZeroSample", {}, "ignore these FEBs for ZeroSample" };

  /**  Minimum number of FEBs in error to trigger EventInfo::LArError 
       Defined as 1 by default/bulk, 4 in online/express in CaloCellGetter (CaloRec package)
  */
  Gaudi::Property<int> m_minFebsInError{this,"minFebInError",1,
        "Minimum number of FEBs in error to trigger EventInfo::LArError (1 by default/bulk, 4 in online/express"}; 

  SG::ReadCondHandleKey<LArBadFebCont> m_bfKey{this,"BFKey","LArBadFeb","Key of the BadFebContainer in the conditions store"};
  SG::ReadHandleKey<LArFebHeaderContainer> m_readKey{this,"ReadKey","LArFebHeader"};
  SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{this,"EventInfoKey","EventInfo"};
  SG::WriteDecorHandleKey<xAOD::EventInfo> m_eventInfoDecorKey{this, "EventInfoDecorKey", m_eventInfoKey, "larFlags"};
  SG::WriteHandleKey<LArFebErrorSummary> m_writeKey{this,"WriteKey","LArFebErrorSummary"};

  // methods:
  static bool masked (unsigned int hid, const std::set<unsigned int>& v_feb) ; 
};
#endif










