/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "PixelClusterAnalysisAlg.h"
#include "AthenaMonitoringKernel/Monitored.h"

namespace ActsTrk {

  PixelClusterAnalysisAlg::PixelClusterAnalysisAlg(const std::string& name, ISvcLocator *pSvcLocator)
    : AthMonitorAlgorithm(name, pSvcLocator) 
  {}
  
  StatusCode PixelClusterAnalysisAlg::initialize() {
    ATH_MSG_DEBUG( "Initializing " << name() << " ... " );
    
    ATH_CHECK( m_pixelClusterContainerKey.initialize() );

    ATH_CHECK(detStore()->retrieve(m_pixelID,"PixelID"));

    ATH_MSG_DEBUG("Monitoring settings ...");
    ATH_MSG_DEBUG(m_monGroupName);
    
    return AthMonitorAlgorithm::initialize();
  }

  StatusCode PixelClusterAnalysisAlg::fillHistograms(const EventContext& ctx) const {
    ATH_MSG_DEBUG( "In " << name() << "::fillHistograms()" );
    
    SG::ReadHandle< xAOD::PixelClusterContainer > inputPixelClusterContainer( m_pixelClusterContainerKey, ctx );
    if (!inputPixelClusterContainer.isValid()){
        ATH_MSG_FATAL("xAOD::PixelClusterContainer with key " << m_pixelClusterContainerKey.key() << " is not available...");
        return StatusCode::FAILURE;
    }

    auto monitor_barrelEndcap = Monitored::Collection("barrelEndcap", *inputPixelClusterContainer,
						      [this] (const auto* cluster) -> int
						      {
							const Identifier& id = m_pixelID->wafer_id(cluster->identifierHash());
							return m_pixelID->barrel_ec(id);
						      });
    auto monitor_layerDisk = Monitored::Collection("layerDisk", *inputPixelClusterContainer,
						      [this] (const auto* cluster) -> int
						   { 
						     const Identifier& id = m_pixelID->wafer_id(cluster->identifierHash());
						     return m_pixelID->layer_disk(id);
						   });
    auto monitor_phiModule = Monitored::Collection("phiModule", *inputPixelClusterContainer,
						      [this] (const auto* cluster) -> int
						      {
							const Identifier& id = m_pixelID->wafer_id(cluster->identifierHash());
							return m_pixelID->phi_module(id);
						      });
    auto monitor_etaModule = Monitored::Collection("etaModule", *inputPixelClusterContainer,
						      [this] (const auto* cluster) -> int
						      {
							const Identifier& id = m_pixelID->wafer_id(cluster->identifierHash());
							return m_pixelID->eta_module(id);
						      });
    auto monitor_isInnermost = Monitored::Collection("isInnermost", *inputPixelClusterContainer,
						      [this] (const auto* cluster) -> int
						      {
							const Identifier& id = m_pixelID->wafer_id(cluster->identifierHash());
							int pixLayerDisk = m_pixelID->layer_disk(id);
							return int(pixLayerDisk==0);
						      });
    auto monitor_isNextToInnermost = Monitored::Collection("isNextToInnermost", *inputPixelClusterContainer,
						      [this] (const auto* cluster) -> int
						      {
							const Identifier& id = m_pixelID->wafer_id(cluster->identifierHash());
							int pixLayerDisk = m_pixelID->layer_disk(id);
							int pixBrlEc = m_pixelID->barrel_ec(id);
							return int((pixLayerDisk==1) or (pixBrlEc!=0 and pixLayerDisk==2));
						      });

    auto monitor_eta = Monitored::Collection("eta", *inputPixelClusterContainer,
					     [] (const auto* cluster) -> double
					     { 
					       const auto& globalPos = cluster->globalPosition();
					       Amg::Vector3D globalPosition(globalPos(0, 0), globalPos(1, 0), globalPos(2, 0));
					       return globalPosition.eta();
					     });
    auto monitor_perp = Monitored::Collection("perp", *inputPixelClusterContainer,
					     [] (const auto* cluster) -> float
					     { 
					       const auto& globalPos = cluster->globalPosition();
					       Amg::Vector3D globalPosition(globalPos(0, 0), globalPos(1, 0), globalPos(2, 0));
					       return globalPosition.perp();
					     });

    auto monitor_globalX = Monitored::Collection("globalX", *inputPixelClusterContainer,
						 [] (const auto* cluster) -> float
						 { return cluster->globalPosition()(0, 0); });
    auto monitor_globalY = Monitored::Collection("globalY", *inputPixelClusterContainer,
						 [] (const auto* cluster) -> float
						 { return cluster->globalPosition()(1, 0); });
    auto monitor_globalZ = Monitored::Collection("globalZ", *inputPixelClusterContainer,
						 [] (const auto* cluster) -> float
						 { return cluster->globalPosition()(2, 0); });

    auto monitor_localX = Monitored::Collection("localX", *inputPixelClusterContainer,
						[] (const auto cluster) -> float
						{ 
						  const auto& localPos = cluster->template localPosition<2>(); 
						  return localPos(0,0); 
						});
    auto monitor_localY = Monitored::Collection("localY", *inputPixelClusterContainer,
						[] (const auto cluster) -> float
						{
						  const auto& localPos = cluster->template localPosition<2>();
						  return localPos(1,0);
						});

    auto monitor_localCovXX = Monitored::Collection("localCovXX", *inputPixelClusterContainer,
						    [] (const auto* cluster) -> float 
						    { return cluster->template localCovariance<2>()(0, 0); });
    auto monitor_localCovYY = Monitored::Collection("localCovYY", *inputPixelClusterContainer,
						    [] (const auto* cluster) -> float 
						    { return cluster->template localCovariance<2>()(1, 1); });
    
    auto monitor_sizeX = Monitored::Collection("sizeX", *inputPixelClusterContainer,
					       [] (const auto* cluster) -> int
					       { return cluster->channelsInPhi(); }); 
    auto monitor_sizeY = Monitored::Collection("sizeY", *inputPixelClusterContainer,
					       [] (const auto* cluster) -> int					       
					       { return cluster->channelsInEta(); });

    auto monitor_widthY = Monitored::Collection("widthY", *inputPixelClusterContainer,
						[] (const auto* cluster) -> float
						{ return cluster->widthInEta(); });

    fill(m_monGroupName.value(),
	 monitor_barrelEndcap, monitor_layerDisk,
	 monitor_phiModule, monitor_etaModule,
	 monitor_isInnermost, monitor_isNextToInnermost,
	 monitor_eta, monitor_perp,
	 monitor_globalX, monitor_globalY, monitor_globalZ,
	 monitor_localX, monitor_localY,
	 monitor_localCovXX, monitor_localCovYY,
	 monitor_sizeX, monitor_sizeY,
	 monitor_widthY);
    
    return StatusCode::SUCCESS;
  }

}
