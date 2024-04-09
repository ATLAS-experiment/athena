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

//stl includes                                                                                              
#include <string>

class NSWDataMonAlg: public AthMonitorAlgorithm {
 public:

  NSWDataMonAlg( const std::string& name, ISvcLocator* pSvcLocator );

  virtual ~NSWDataMonAlg()=default;
  virtual StatusCode initialize() override;
  virtual StatusCode fillHistograms(const EventContext& ctx) const override;
  
 private:

  Gaudi::Property<bool> m_doESD{this,"DoESD",true};
  SG::ReadHandleKey<xAOD::MuonContainer> m_muonKey{this, "MuonsKey", "Muons"};
  
};    
#endif
