/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "HIClusterGeoWeights/HIClusterGeo_HistoFiller.h"

HIClusterGeo_HistoFiller::HIClusterGeo_HistoFiller(const std::string& name, ISvcLocator* pSvcLocator) :
  AthAlgorithm(name, pSvcLocator), m_etaPhiMapping(nullptr)
{
}

TH1 * HIClusterGeo_HistoFiller::regAndGetTHF(const std::string& histName, const std::string& histTitle, int numBinsX, double xMin, double xMax, int numBinsY, double yMin, double yMax)
{
  auto hist = std::make_unique<TH2F>(histName.c_str(), histTitle.c_str(), numBinsX, xMin, xMax, numBinsY, yMin, yMax);
  if(m_thistSvc->regHist("/" + m_histStream + "/" + histName, std::unique_ptr<TH1>(std::move(hist))).isFailure())
  {
    return nullptr;
  }

  TH1 * tmpHist = nullptr;
  if(m_thistSvc->getHist("/" + m_histStream + "/" + histName, tmpHist).isFailure())
  {
    return nullptr;
  }

  return static_cast<TH1*>(tmpHist);
}

StatusCode HIClusterGeo_HistoFiller::initialize()
{
  ATH_CHECK( m_eventInfoKey.initialize() );
  ATH_CHECK( m_vertexContainerKey.initialize() );
  ATH_CHECK( m_caloClusterContainerKey.initialize() );
  ATH_CHECK( m_hiEventShapeKey.initialize() );

  ATH_CHECK( m_thistSvc.retrieve() );
  
  m_histTileWeights.clear();

  m_etaPhiMapping = static_cast<TH2F*>(regAndGetTHF("h_etaPhiMapping", ";eta;phi", m_etaBins, -5, 5, m_phiBins, -TMath::Pi(), TMath::Pi()));
  if(m_etaPhiMapping==nullptr)
  {
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

StatusCode HIClusterGeo_HistoFiller::execute()
{
  const EventContext& ctx = Gaudi::Hive::currentContext();

  SG::ReadHandle<xAOD::EventInfo> eventInfo{m_eventInfoKey, ctx};
  if (!eventInfo.isValid())
  {
    ATH_MSG_WARNING("Cannot get EventInfo with key " << m_eventInfoKey.key());
    return StatusCode::SUCCESS;
  }

  if((eventInfo->errorState(xAOD::EventInfo::LAr) == xAOD::EventInfo::Error)  ||
     (eventInfo->errorState(xAOD::EventInfo::Tile) == xAOD::EventInfo::Error) ||
     (eventInfo->isEventFlagBitSet(xAOD::EventInfo::Core, 18)) )
  {
    return StatusCode::SUCCESS;
  }

  // GRL selection will be done later by "makeHIResponse",
  // here, we just save everything into separate histograms for each LB

  // trigger decision tool is not needed if we use CC and PC streams

  SG::ReadHandle<xAOD::VertexContainer> vertices{m_vertexContainerKey, ctx};
  if(!vertices.isValid())
  {
    ATH_MSG_WARNING("Cannot get VertexContainer with key " << m_vertexContainerKey.key());
    return StatusCode::SUCCESS;
  }
  if(vertices->size() <= 1)
  {
    // no vertex
    return StatusCode::SUCCESS;
  }

  SG::ReadHandle<xAOD::CaloClusterContainer> caloClusters{m_caloClusterContainerKey, ctx};
  if (!caloClusters.isValid())
  {
    ATH_MSG_WARNING("Cannot get CaloClusterContainer with key " << m_caloClusterContainerKey.key());
    return StatusCode::SUCCESS;
  }

  SG::ReadHandle<xAOD::HIEventShapeContainer> hiEventShape{m_hiEventShapeKey, ctx};
  if (!hiEventShape.isValid())
  {
    ATH_MSG_WARNING("Cannot get HIEventShapeContainer with key " << m_hiEventShapeKey.key());
    return StatusCode::SUCCESS;
  }

  int currentLB = eventInfo->lumiBlock();
  int currentRun = eventInfo->runNumber();

  if (auto search = m_histTileWeights.find(currentLB); search == m_histTileWeights.end())
  {
    // if m_histTileWeights does not already have histogram forthis LB, allocate a new TH2F
    // bins in Y: sum FCalET, sum ClusterET, sum FCalET^2, sum ClusterET^2, sum FCalET*ClusterET, number of events
    TH2F * newHistTileWeights = static_cast<TH2F*>(regAndGetTHF(Form("h_clusterET_fcalET_%d_%d",currentRun,currentLB), ";bin ID;sum FCalET/clusterET/FCalET^2/clusterET^2/FCalET*clusterET/n_{events}", m_totalBins, -0.5, m_totalBins-0.5, 6,0.5,6.5));
    if (newHistTileWeights == nullptr)
    {
      ATH_MSG_WARNING("Could not create histogram "<<Form("h_clusterET_fcalET_%d_%d",currentRun,currentLB)<<" that was supposed to be used for Run "<<currentRun<<" LB "<<currentLB);
      return StatusCode::SUCCESS;
    }

    m_histTileWeights[currentLB] = std::move(newHistTileWeights); 
  }

  auto currentHistTileWeights = m_histTileWeights.at(currentLB);
  
  float fcalEt = hiEventShape->at(5)->et()*1e-6;
  if(fcalEt<m_minFCalET || m_maxFCalET<fcalEt)
  {
    // remove in-time and out-of-time pile-up
    return StatusCode::SUCCESS;
  }

  for(const auto* cluster : *caloClusters)
  {
    float eta = cluster->eta();
    float phi = cluster->phi();
    int binID = m_etaPhiMapping->FindBin(eta, phi);
    float ET = cluster->e() * 1e-3 / std::cosh(eta);

    currentHistTileWeights->Fill(binID, 1, fcalEt);
    currentHistTileWeights->Fill(binID, 2, ET);
    currentHistTileWeights->Fill(binID, 3, fcalEt*fcalEt);
    currentHistTileWeights->Fill(binID, 4, ET*ET);
    currentHistTileWeights->Fill(binID, 5, fcalEt*ET);
    currentHistTileWeights->Fill(binID, 6, 1.);
  }
  
  return StatusCode::SUCCESS;
}

StatusCode HIClusterGeo_HistoFiller::finalize()
{ 
  return StatusCode::SUCCESS;
}
