/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HICLUSTERGEOWEIGHTS_HICLUSTERGEO_HISTOFILLER_H
#define HICLUSTERGEOWEIGHTS_HICLUSTERGEO_HISTOFILLER_H

// Framework includes
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ITHistSvc.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODHIEvent/HIEventShapeContainer.h"
#include "xAODTracking/VertexContainer.h"

// STL includes
#include <memory> // for unique_ptr
#include <unordered_map>

//ROOT includes
#include "TH2F.h"
#include "TMath.h"


class HIClusterGeo_HistoFiller : public AthAlgorithm {
public:
  HIClusterGeo_HistoFiller(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~HIClusterGeo_HistoFiller() = default;

  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;
  virtual StatusCode finalize() override;

private:
  ServiceHandle<ITHistSvc> m_thistSvc{this, "THistSvc", "THistSvc"};
  Gaudi::Property<std::string> m_histStream{this, "HistStream", "CLUSTERGEOFILLERSTREAM"};
  Gaudi::Property<float> m_minFCalET{this, "minFCalET", 0.0, "minimum allowed FCal ET in TeV"};
  Gaudi::Property<float> m_maxFCalET{this, "maxFCalET", 5.4, "maximum allowed FCal ET in TeV"};

  SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{this, "EventInfoKey", "EventInfo", "name of EventInfo container"};
  SG::ReadHandleKey<xAOD::VertexContainer> m_vertexContainerKey{this, "VertexContainerKey", "PrimaryVertices", "name of VertexContainer"};
  SG::ReadHandleKey<xAOD::CaloClusterContainer> m_caloClusterContainerKey{this, "CaloClusterContainerKey", "HIClusters", "name of CaloClusterContainer"};
  SG::ReadHandleKey<xAOD::HIEventShapeContainer> m_hiEventShapeKey{this, "HIEventShapeKey", "CaloSums", "name of HIEventShapeContainer"};

  std::unordered_map<int, TH2F*> m_histTileWeights;
  const int m_etaBins=100, m_phiBins=64;
  const int m_totalBins = (m_etaBins+2)*(m_phiBins+2);
  TH2F * m_etaPhiMapping;

  TH1 * regAndGetTHF(const std::string& histName, const std::string& histTitle, int numBinsX, double xMin, double xMax, int numBinsY, double yMin, double yMax);
};


// generate with `clid -m "HIClusterGeo_HistoFiller" `
#ifndef __CINT__
  CLASS_DEF( HIClusterGeo_HistoFiller , 60054236 , 1 )
#endif

#endif // HICLUSTERGEOWEIGHTS_HICLUSTERGEO_HISTOFILLER_H
