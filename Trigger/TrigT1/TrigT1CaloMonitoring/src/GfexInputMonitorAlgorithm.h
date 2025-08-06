/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGT1CALOMONITORING_GFEXINPUTMONITORALGORITHM_H
#define TRIGT1CALOMONITORING_GFEXINPUTMONITORALGORITHM_H

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "StoreGate/ReadHandleKey.h"

//
#include "xAODTrigL1Calo/gFexTowerContainer.h"
#include "xAODTrigL1Calo/gFexTower.h"

class GfexInputMonitorAlgorithm : public AthMonitorAlgorithm {
public:GfexInputMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator );
  virtual ~GfexInputMonitorAlgorithm()=default;
  virtual StatusCode initialize() override;
  virtual StatusCode fillHistograms( const EventContext& ctx ) const override;

private:

  StringProperty m_packageName{this,"PackageName","GfexInputMonitor","group name for histograming"};

  // container keys including steering parameter and description
  SG::ReadHandleKey<xAOD::gFexTowerContainer> m_gFexTowerContainerKey{this, "gFexTowerContainer","L1_gFexDataTowers","SG key of the input gFex Tower container"};
  SG::ReadHandleKey<xAOD::gFexTowerContainer> m_gFexEmulatedTowerKey{this, "gFexEmulatedTowerContainer","L1_gFexEmulatedTowers","SG key of the emulated gFex Tower container"};

    //funtions defined
  int getBinNumberTower(const float& inputEta, const float& inputPhi, int xbin, int ybin) const;

};
#endif
