/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARCELLREC_LARNOISYROALG_H
#define LARCELLREC_LARNOISYROALG_H

/** 
@class LArNoisyROAlg
@brief Find list of suspicious preamplifiers and Front End Boards from cell collection

 Created September 28, 2009  L. Duflot
 Modified May, 2014 B.Trocme 
 - Remove saturated medium cut
 - Create a new weighted Std algorithm
 Modified October 2024 P. Strizenec
 - added noisy HV lines flagging

*/



#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "CaloInterface/ILArNoisyROTool.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "LArRecConditions/LArBadChannelCont.h"
#include "LArRecConditions/LArHVNMap.h"
#include "LArRecConditions/LArHVIdMapping.h"
#include "LArRecEvent/LArNoisyROSummary.h"
#include "xAODEventInfo/EventInfo.h"

class CaloCellContainer;

class LArNoisyROAlg : public AthReentrantAlgorithm
{
 public:

  LArNoisyROAlg(const std::string &name,ISvcLocator *pSvcLocator);
  virtual StatusCode initialize() override;
  virtual StatusCode execute (const EventContext& ctx) const override;   
  virtual StatusCode finalize() override;

  typedef std::map<int, int> hvmap_type;
 
 private: 
  ToolHandle<ILArNoisyROTool> m_noisyROTool;

  Gaudi::Property<bool>  m_isMC     { this, "isMC", false, "Are we working with simu?" };
  SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{this,"EventInfoKey","EventInfo"};
  SG::WriteDecorHandleKey<xAOD::EventInfo> m_eventInfoDecorKey{this,"EventInfoDecorKey","EventInfo.larFlags"};
  SG::ReadHandleKey<CaloCellContainer> m_CaloCellContainerName {this, "CaloCellContainer", "AllCalo", "input cell container key"};
  SG::WriteHandleKey<LArNoisyROSummary> m_outputKey {this, "OutputKey", "LArNoisyROSummary", "output object key"};
  SG::ReadCondHandleKey<LArBadFebCont> m_knownBadFEBsVecKey {this, "BadFEBsKey", "LArKnownBadFEBs", "key to read the known Bad FEBs"};
  SG::ReadCondHandleKey<LArBadFebCont> m_knownMNBFEBsVecKey {this, "MNBFEBsKey", "LArKnownMNBFEBs", "key to read the known MNB FEBs"};
  SG::ReadCondHandleKey<LArHVNMap> m_hvMapKey {this, "HVMapKey", "", "key to read HVline Ncells map"};
  SG::ReadCondHandleKey<CaloDetDescrManager> m_caloDetDescrMgrKey{this,"CaloDetDescrManager", "CaloDetDescrManager"};
  SG::ReadCondHandleKey<LArHVIdMapping> m_hvCablingKey {this, "LArHVIdMapping", "", "SG key for HV ID mapping"};

};


#endif
