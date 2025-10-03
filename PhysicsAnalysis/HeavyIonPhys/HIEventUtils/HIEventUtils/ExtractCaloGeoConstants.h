/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HIEVENTUTILS_EXTRACTCALOGEOCONSTANTS_H
#define HIEVENTUTILS_EXTRACTCALOGEOCONSTANTS_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "CaloEvent/CaloCellContainer.h"
#include "CaloEvent/CaloTowerContainer.h"
#include "Navigation/NavigationToken.h"
#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/ServiceHandle.h"

#include <TMath.h>
#include <TH3F.h>

class ExtractCaloGeoConstants : public AthAlgorithm
{

public:

  ExtractCaloGeoConstants(const std::string& name, ISvcLocator* pSvcLocator);
  ~ExtractCaloGeoConstants() = default;

  virtual StatusCode initialize();
  virtual StatusCode execute();
  virtual StatusCode finalize();

private:
  SG::ReadHandleKey<CaloTowerContainer> m_tower_container_key{ this, "InputTowerKey", "CombinedTower", "name of input CaloTowerContainer"};
  SG::ReadHandleKey<CaloCellContainer> m_cell_container_key{ this, "CaloCellContainerKey", "AllCalo", "name of input CaloCellContainer"};

  ServiceHandle<ITHistSvc> m_thistSvc{this, "THistSvc", "THistSvc"};
  Gaudi::Property<std::string> m_hist_stream{this, "HistStream", "CALOGEOEXTRACTSTREAM"};

  TH3F* m_h3_w{nullptr};
  TH3F* m_h3_eta{nullptr};
  TH3F* m_h3_phi{nullptr};
  TH3F* m_h3_R{nullptr};
  TH1F* m_h1_events{nullptr};
};
#endif
