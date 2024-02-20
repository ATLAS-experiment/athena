/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "HgtdClusterAnalysisAlg.h"
#include "AthenaMonitoringKernel/Monitored.h"

namespace ActsTrk {

  HgtdClusterAnalysisAlg::HgtdClusterAnalysisAlg(const std::string& name, ISvcLocator *pSvcLocator)
    : AthMonitorAlgorithm(name, pSvcLocator) 
  {}
  
  StatusCode HgtdClusterAnalysisAlg::initialize() {
    ATH_MSG_DEBUG( "Initializing " << name() << " ... " );
    
    ATH_CHECK( m_hgtdClusterContainerKey.initialize() );

    ATH_CHECK(detStore()->retrieve(m_hgtdID,"HGTD_ID"));

    ATH_MSG_DEBUG("Monitoring settings ...");
    ATH_MSG_DEBUG(m_monGroupName);
    
    return AthMonitorAlgorithm::initialize();
  }

  StatusCode HgtdClusterAnalysisAlg::fillHistograms(const EventContext& ctx) const {
    ATH_MSG_DEBUG( "In " << name() << "::fillHistograms()" );

    SG::ReadHandle< xAOD::HGTDClusterContainer > inputHgtdClusterContainer( m_hgtdClusterContainerKey, ctx );
    if (not inputHgtdClusterContainer.isValid()){
        ATH_MSG_FATAL("xAOD::HGTDClusterContainer with key " << m_hgtdClusterContainerKey.key() << " is not available...");
        return StatusCode::FAILURE;
    }

    // IDs variables, e.g. module, disk or any detector-related variable HGTD uses
    // We can add global position here
    // Variables like eta, perp
    
    // Local Position
    auto monitor_localX = Monitored::Collection("localX", *inputHgtdClusterContainer,
						[] (const auto cluster) -> float
						{ 
						  const auto& localPos = cluster->template localPosition<3>(); 
						  return localPos(0,0); 
						});
    auto monitor_localY = Monitored::Collection("localY", *inputHgtdClusterContainer,
						[] (const auto cluster) -> float
						{
						  const auto& localPos = cluster->template localPosition<3>();
						  return localPos(1,0);
						});

    auto monitor_localT = Monitored::Collection("localT", *inputHgtdClusterContainer,
                                                [] (const auto cluster) -> float
                                                {
                                                  const auto& localPos = cluster->template localPosition<3>();
                                                  return localPos(2,0);
                                                });

    // Local Covariance
    auto monitor_localCovXX = Monitored::Collection("localCovXX", *inputHgtdClusterContainer,
						    [] (const auto* cluster) -> float 
						    { return cluster->template localCovariance<3>()(0, 0); });
    auto monitor_localCovYY = Monitored::Collection("localCovYY", *inputHgtdClusterContainer,
						    [] (const auto* cluster) -> float 
						    { return cluster->template localCovariance<3>()(1, 1); });

    auto monitor_localCovTT = Monitored::Collection("localCovTT", *inputHgtdClusterContainer,
                                                    [] (const auto* cluster) -> float
                                                    { return cluster->template localCovariance<3>()(2, 2); });
    
    fill(m_monGroupName.value(),
	 monitor_localX, monitor_localY, monitor_localT,
	 monitor_localCovXX, monitor_localCovYY, monitor_localCovTT);
    return StatusCode::SUCCESS;
  }

}
