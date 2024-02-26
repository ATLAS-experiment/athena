/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "HgtdClusterAnalysisAlg.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "HGTD_ReadoutGeometry/HGTD_DetectorElement.h"

namespace ActsTrk {

  HgtdClusterAnalysisAlg::HgtdClusterAnalysisAlg(const std::string& name, ISvcLocator *pSvcLocator)
    : AthMonitorAlgorithm(name, pSvcLocator) 
  {}
  
  StatusCode HgtdClusterAnalysisAlg::initialize() {
    ATH_MSG_DEBUG( "Initializing " << name() << " ... " );
    
    ATH_CHECK( m_hgtdClusterContainerKey.initialize() );
    ATH_CHECK( m_HGTDDetEleCollKey.initialize() );
    ATH_CHECK(detStore()->retrieve(m_hgtdID,"HGTD_ID"));

    ATH_MSG_DEBUG("Monitoring settings ...");
    ATH_MSG_DEBUG(m_monGroupName);
    
    return AthMonitorAlgorithm::initialize();
  }

  StatusCode HgtdClusterAnalysisAlg::fillHistograms(const EventContext& ctx) const {
    ATH_MSG_DEBUG( "In " << name() << "::fillHistograms()" );

    SG::ReadCondHandle<InDetDD::HGTD_DetectorElementCollection> hgtdDetEleHandle(m_HGTDDetEleCollKey, ctx);
    const InDetDD::HGTD_DetectorElementCollection* hgtdElements(*hgtdDetEleHandle);
    if (not hgtdDetEleHandle.isValid() or hgtdElements==nullptr) {
      ATH_MSG_FATAL(m_HGTDDetEleCollKey.fullKey() << " is not available.");
      return StatusCode::FAILURE;
    }
    
    SG::ReadHandle< xAOD::HGTDClusterContainer > inputHgtdClusterContainer( m_hgtdClusterContainerKey, ctx );
    if (not inputHgtdClusterContainer.isValid()){
        ATH_MSG_FATAL("xAOD::HGTDClusterContainer with key " << m_hgtdClusterContainerKey.key() << " is not available...");
        return StatusCode::FAILURE;
    }
    const xAOD::HGTDClusterContainer* hgtdClusterContainer = inputHgtdClusterContainer.cptr();
    
    // IDs variables, e.g. module, disk or any detector-related variable HGTD uses
    // We can add global position here
    // Variables like eta, perp

    std::vector<float> global_x (hgtdClusterContainer->size(), 0);
    std::vector<float> global_y (hgtdClusterContainer->size(), 0);
    std::vector<float> global_z (hgtdClusterContainer->size(), 0);
    std::vector<float> global_r (hgtdClusterContainer->size(), 0);
    std::vector<float> eta (hgtdClusterContainer->size(), 0);

    for (std::size_t i(0ul); i<hgtdClusterContainer->size(); ++i) {
      const xAOD::HGTDCluster *cluster = hgtdClusterContainer->at(i);
      const Identifier& id = m_hgtdID->wafer_id(cluster->identifierHash());
      const auto *element = hgtdElements->getDetectorElement(m_hgtdID->wafer_hash(m_hgtdID->wafer_id(id)));

      const Amg::Transform3D& T = element->surface().transform();
      double Ax[3] = {T(0,0),T(1,0),T(2,0)};
      double Ay[3] = {T(0,1),T(1,1),T(2,1)};
      double R [3] = {T(0,3),T(1,3),T(2,3)};

      const auto& local_position = cluster->template localPosition<3>();
      Amg::Vector2D M;
      M[0] = local_position(0,0);
      M[1] = local_position(1,0);
      Amg::Vector3D globalPos(M[0]*Ax[0]+M[1]*Ay[0]+R[0],M[0]*Ax[1]+M[1]*Ay[1]+R[1],M[0]*Ax[2]+M[1]*Ay[2]+R[2]);

      global_x[i] = globalPos.x();
      global_y[i] = globalPos.y();
      global_z[i] = globalPos.z();
      global_r[i] = std::sqrt(global_x[i]*global_x[i] + global_y[i]*global_y[i]);
      eta[i] = globalPos.eta();
    }
    
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

    auto monitor_globalX = Monitored::Collection("globalX", global_x);
    auto monitor_globalY = Monitored::Collection("globalY", global_y);
    auto monitor_globalZ = Monitored::Collection("globalZ", global_z);
    auto monitor_globalR = Monitored::Collection("globalR", global_r);
    auto monitor_eta = Monitored::Collection("eta", eta);
    
    fill(m_monGroupName.value(),
	 monitor_localX, monitor_localY, monitor_localT,
	 monitor_localCovXX, monitor_localCovYY, monitor_localCovTT,
	 monitor_globalX, monitor_globalY, monitor_globalZ, monitor_globalR,
	 monitor_eta);
    return StatusCode::SUCCESS;
  }

}
