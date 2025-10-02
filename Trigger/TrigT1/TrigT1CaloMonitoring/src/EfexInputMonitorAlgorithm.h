/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGT1CALOMONITORING_EFEXINPUTMONITORALGORITHM_H
#define TRIGT1CALOMONITORING_EFEXINPUTMONITORALGORITHM_H

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "StoreGate/ReadHandleKey.h"

#include "LArRecConditions/LArBadChannelCont.h"
//
#include "xAODTrigL1Calo/eFexTowerContainer.h"
#include "xAODTrigL1Calo/eFexTower.h"

#include "LArCabling/LArOnOffIdMapping.h"



class EfexInputMonitorAlgorithm : public AthMonitorAlgorithm {
public:EfexInputMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator );
  virtual ~EfexInputMonitorAlgorithm()=default;
  virtual StatusCode initialize() override;
  virtual StatusCode fillHistograms( const EventContext& ctx ) const override;

private:

  StringProperty m_packageName{this,"PackageName","EfexInputMonitor","group name for histograming"};

  // container keys including steering parameter and description
  SG::ReadHandleKey<xAOD::eFexTowerContainer> m_eFexTowerContainerKey{this, "eFexTowerContainer","L1_eFexDataTowers","SG key of the input eFex Tower container"};
  SG::ReadHandleKey<xAOD::eFexTowerContainer> m_eFexTowerContainerRefKey{this,"eFexTowerContainerRef","L1_eFexEmulatedTowers","SG key of the towers to use as a reference (defaults to emulated towers built from sCells and tTowers"};

  std::map<std::pair<std::pair<int,int>,int>,std::pair<std::set<unsigned long long>,std::string>> m_scMap;

  // some counters to avoid filling debugging histograms more than 20 times
  mutable std::atomic<size_t> m_debugEvtCount = 0;
  mutable std::atomic<size_t> m_debugEvtCount2 = 0;


    SG::ReadCondHandleKey<LArBadChannelCont> m_bcContKey{this, "LArMaskedChannelKey", "LArMaskedSC", "Key of the OTF-Masked SC" };

};
#endif
