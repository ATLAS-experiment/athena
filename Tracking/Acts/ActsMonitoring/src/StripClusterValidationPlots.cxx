/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "src/StripClusterValidationPlots.h"

namespace ActsTrk {

  StripClusterValidationPlots::StripClusterValidationPlots(PlotBase* pParent, 
							   const std::string& sDir)
    : PlotBase(pParent, sDir)
  {
    m_barrelEndcap = Book1D("barrelEndcap", "StripCluster_barrelEndcap;Barrel-Endcap;Entries;", 5, -2, 3, false);

    m_layerDisk_barrel = Book1D("layerDisk_barrel", "StripCluster_layerDisk_barrel;Layer Disk;Entries;", 6, 0, 6, false);
    m_layerDisk_endcap = Book1D("layerDisk_endcap", "StripCluster_layerDisk_endcap;Layer Disk;Entries;", 6, 0, 6, false);

    m_phiModule_barrel = Book1D("phiModule_barrel", "StripCluster_phiModule_barrel;Phi Module;Entries", 75, 0, 75, false);
    m_phiModule_endcap = Book1D("phiModule_endcap", "StripCluster_phiModule_endcap;Phi Module;Entries", 75, 0, 75, false);

    m_etaModule_barrel = Book1D("etaModule_barrel", "StripCluster_etaModule_barrel;Eta Module;Entries;", 120, -60, 60, false);
    m_etaModule_endcap = Book1D("etaModule_endcap", "StripCluster_etaModule_endcap;Eta Module;Entries;", 120, -60, 60, false);

    m_sideModule_barrel = Book1D("sideModule_barrel", "StripCluster_sideModule_barrel;Side Module;Entries;", 2, 0, 2, false);
    m_sideModule_endcap = Book1D("sideModule_endcap", "StripCluster_sideModule_endcap;Side Module;Entries;", 2, 0, 2, false);

    m_eta_barrel = Book1D("eta_barrel", "StripCluster_eta_barrel;eta;Entries;", 30, -3, 3, false);
    m_eta_endcap = Book1D("eta_endcap", "StripCluster_eta_endcap;eta;Entries;", 30, -3, 3, false);

    m_perp_barrel = Book1D("perp_barrel", "StripCluster_perp_barrel;r [mm];Entries;", 100, 300, 1100, false);
    m_perp_endcap = Book1D("perp_endcap", "StripCluster_perp_endcap;r [mm];Entries;", 100, 300, 1100, false);

    m_global_x_barrel = Book1D("global_x_barrel", "StripCluster_global_x_barrel;Global x [mm];Entries;", 100, -1100, 1100, false);
    m_global_x_endcap = Book1D("global_x_endcap", "StripCluster_global_x_endcap;Global x [mm];Entries;", 100, -1100, 1100, false);

    m_global_y_barrel = Book1D("global_y_barrel", "StripCluster_global_y_barrel;Global y [mm];Entries;", 100, -1100, 1100, false);
    m_global_y_endcap = Book1D("global_y_endcap", "StripCluster_global_y_endcap;Global y [mm];Entries;", 100, -1100, 1100, false);

    m_global_z_barrel = Book1D("global_z_barrel", "StripCluster_global_z_barrel;Global z [mm];Entries;", 100, -3000, 3000, false);
    m_global_z_endcap = Book1D("global_z_endcap", "StripCluster_global_z_endcap;Global z [mm];Entries;", 100, -3000, 3000, false);
    
    m_local_x_barrel = Book1D("local_x_barrel", "StripCluster_local_x_barrel;Loval x [mm];Entries;", 100, -50, 50, false);
    m_local_x_endcap = Book1D("local_x_endcap", "StripCluster_local_x_endcap;Loval x [mm];Entries;", 100, -50, 50, false);

    m_localCovXX_barrel = Book1D("localCovXX_barrel", "StripCluster_localCovXX_barrel;Local Cov XX [mm2];Entries;", 100, 0, 0.5*1e-3, false);
    m_localCovXX_endcap = Book1D("localCovXX_endcap", "StripCluster_localCovXX_endcap;Local Cov XX [mm2];Entries;", 100, 0, 0.5*1e-3, false);

    m_sizeX_barrel = Book1D("sizeX_barrel", "StripCluster_sizeX_barrel;Size X;Entries;", 100, 0, 400, false);
    m_sizeX_endcap = Book1D("sizeX_endcap", "StripCluster_sizeX_endcap;Size X;Entries;", 100, 0, 400, false);

    m_global_xy_barrel = Book2D("global_xy_barrel", "StripCluster_global_xy_barrel;x [mm];y [mm];", 100, -1100, 1100, 100, -1100, 1100, false);
    m_global_xy_endcap = Book2D("global_xy_endcap", "StripCluster_global_xy_endcap;x [mm];y [mm];", 100, -1100, 1100, 100, -1100, 1100, false);

    m_global_zr_barrel = Book2D("global_zr_barrel", "StripCluster_global_zr_barrel;z [mm];r [mm];", 100, -3000, 3000, 100, 300, 1100, false);
    m_global_zr_endcap = Book2D("global_zr_endcap", "StripCluster_global_zr_endcap;z [mm];r [mm];", 100, -3000, 3000, 100, 300, 1100, false);
  }

  void StripClusterValidationPlots:: fill(const xAOD::StripCluster* cluster,
					  float beamSpotWeight,
					  const SCT_ID* stripID)
  {
    const Identifier& id = stripID->wafer_id(cluster->identifierHash());
    bool isBarrel = (stripID->barrel_ec(id) == 0);

    const auto& local_position = cluster->template localPosition<1>();
    const auto& local_covariance = cluster->template localCovariance<1>();
    const auto& globalPos = cluster->globalPosition();
    Amg::Vector3D globalPosition(globalPos(0, 0), globalPos(1, 0), globalPos(2, 0));

    m_barrelEndcap->Fill(stripID->barrel_ec(id), beamSpotWeight);

    if (isBarrel) {
      m_layerDisk_barrel->Fill(stripID->layer_disk(id), beamSpotWeight);
      m_phiModule_barrel->Fill(stripID->phi_module(id), beamSpotWeight);
      m_etaModule_barrel->Fill(stripID->eta_module(id), beamSpotWeight);
      m_sideModule_barrel->Fill(stripID->side(id), beamSpotWeight);

      m_eta_barrel->Fill(globalPosition.eta(), beamSpotWeight);
      m_perp_barrel->Fill(globalPosition.perp() , beamSpotWeight);

      m_global_x_barrel->Fill(globalPos(0, 0), beamSpotWeight);
      m_global_y_barrel->Fill(globalPos(1, 0), beamSpotWeight);
      m_global_z_barrel->Fill(globalPos(2, 0), beamSpotWeight);

      m_local_x_barrel->Fill(local_position(0, 0), beamSpotWeight);

      m_localCovXX_barrel->Fill(local_covariance(0, 0), beamSpotWeight);

      m_sizeX_barrel->Fill(cluster->channelsInPhi(), beamSpotWeight);

      m_global_xy_barrel->Fill(globalPos(0, 0), globalPos(1, 0), beamSpotWeight);
      m_global_zr_barrel->Fill(globalPos(2, 0), globalPosition.perp(), beamSpotWeight);
    } else {
      m_layerDisk_endcap->Fill(stripID->layer_disk(id), beamSpotWeight);
      m_phiModule_endcap->Fill(stripID->phi_module(id), beamSpotWeight);
      m_etaModule_endcap->Fill(stripID->eta_module(id), beamSpotWeight);
      m_sideModule_endcap->Fill(stripID->side(id), beamSpotWeight);
      
      m_eta_endcap->Fill(globalPosition.eta(), beamSpotWeight);
      m_perp_endcap->Fill(globalPosition.perp() , beamSpotWeight);

      m_global_x_endcap->Fill(globalPos(0, 0), beamSpotWeight);
      m_global_y_endcap->Fill(globalPos(1, 0), beamSpotWeight);
      m_global_z_endcap->Fill(globalPos(2, 0), beamSpotWeight);

      m_local_x_endcap->Fill(local_position(0, 0), beamSpotWeight);

      m_localCovXX_endcap->Fill(local_covariance(0, 0), beamSpotWeight);

      m_sizeX_endcap->Fill(cluster->channelsInPhi(), beamSpotWeight);

      m_global_xy_endcap->Fill(globalPos(0, 0), globalPos(1, 0), beamSpotWeight);
      m_global_zr_endcap->Fill(globalPos(2, 0), globalPosition.perp(), beamSpotWeight);
    }
  }

}
