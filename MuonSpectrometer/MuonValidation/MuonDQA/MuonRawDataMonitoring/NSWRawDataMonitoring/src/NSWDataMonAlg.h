/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////////////////////////////
// Package : NSWDataMonitoring
// Author:  M. Biglietti (Roma Tre)
//
// DESCRIPTION:
// Subject: MM-->Offline Muon Data Quality
///////////////////////////////////////////////////////////////////////////////////////////

#ifndef NSWDataMonAlg_H
#define NSWDataMonAlg_H

//Core Include
#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "StoreGate/ReadHandleKey.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"


//stl includes                                                                                              
#include <string>

class NSWDataMonAlg: public AthMonitorAlgorithm {
 public:

  NSWDataMonAlg( const std::string& name, ISvcLocator* pSvcLocator );

  virtual ~NSWDataMonAlg()=default;
  virtual StatusCode initialize() override;
  virtual StatusCode fillHistograms(const EventContext& ctx) const override;
  
 private:

  SG::ReadHandleKey<xAOD::MuonContainer> m_muonKey{this, "MuonsKey", "Muons"};
  ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc {this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
  DoubleProperty m_cutPt{this,"muonPtCut",15000,"Minimum muon transverse momentum"};


};    
#endif
