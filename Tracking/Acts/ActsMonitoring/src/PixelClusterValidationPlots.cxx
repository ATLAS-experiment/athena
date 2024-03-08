/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "src/PixelClusterValidationPlots.h"

namespace ActsTrk {

  PixelClusterValidationPlots::PixelClusterValidationPlots(PlotBase* pParent, 
							   const std::string& sDir)
    : PlotBase(pParent, sDir)
  {
    m_barrelEndcap = Book1D("barrelEndcap", "PixelCluster_barrelEndcap;Barrel-Endcap;Entries;", 5, -2, 3, false);

    m_layerDisk_barrel = Book1D("layerDisk_barrel", "PixelCluster_layerDisk_barrel;Layer Disk;Entries;", 9, 0, 9, false);
    m_layerDisk_endcap = Book1D("layerDisk_endcap", "PixelCluster_layerDisk_endcap;Layer Disk;Entries;", 9, 0, 9, false);

    m_phiModule_barrel = Book1D("phiModule_barrel", "PixelCluster_phiModule_barrel;Phi Module;Entries;", 56, 0, 56, false);
    m_phiModule_endcap = Book1D("phiModule_endcap", "PixelCluster_phiModule_endcap;Phi Module;Entries;", 56, 0, 56, false);

    m_etaModule_barrel = Book1D("etaModule_barrel", "PixelCluster_etaModule_barrel;Eta Module;Entries;", 46, -18, 28, false);
    m_etaModule_endcap = Book1D("etaModule_endcap", "PixelCluster_etaModule_endcap;Eta Module;Entries;", 46, -18, 28, false);

    m_isInnermost_barrel = Book1D("isInnermost_barrel", "PixelCluster_isInnermost_barrel;In Innermost;Entries;", 2, 0, 2, false);
    m_isInnermost_endcap = Book1D("isInnermost_endcap", "PixelCluster_isInnermost_endcap;In Innermost;Entries;", 2, 0, 2, false);

    m_isNextToInnermost_barrel = Book1D("isNextToInnermost_barrel", "PixelCluster_isNextToInnermost_barrel;Is Next To Innermost;Entries", 2, 0, 2, false);
    m_isNextToInnermost_endcap = Book1D("isNextToInnermost_endcap", "PixelCluster_isNextToInnermost_endcap;Is Next To Innermost;Entries", 2, 0, 2, false);

    m_eta_barrel = Book1D("eta_barrel", "PixelCluster_eta_barrel;eta;Entries;", 50, -5, 5, false);
    m_eta_endcap = Book1D("eta_endcap", "PixelCluster_eta_endcap;eta;Entries;", 50, -5, 5, false);

    m_perp_barrel = Book1D("perp_barrel", "PixelCluster_perp_barrel;r [mm];Entries;", 100, 0, 320, false);
    m_perp_endcap = Book1D("perp_endcap", "PixelCluster_perp_endcap;r [mm];Entries;", 100, 0, 320, false);

    m_total_charge_barrel = Book1D("total_charge_barrel", "PixelCluster_totalCharge_barrel;charge;Entries;", 50, 0, 300000, false);
    m_total_charge_endcap = Book1D("total_charge_endcap", "PixelCluster_totalCharge_endcap;charge;Entries;", 50, 0, 300000, false);

    m_total_tot_barrel = Book1D("total_tot_barrel", "PixelCluster_totalTot_barrel;charge;Entries;", 50, 0, 5000, false);
    m_total_tot_endcap = Book1D("total_tot_endcap", "PixelCluster_totalTot_endcap;charge;Entries;", 50, 0, 5000, false);

    m_global_x_barrel = Book1D("global_x_barrel", "PixelCluster_global_x_barrel;Global x [mm];Entries;", 64, -350, 350, false);
    m_global_x_endcap = Book1D("global_x_endcap", "PixelCluster_global_x_endcap;Global x [mm];Entries;", 64, -350, 350, false);

    m_global_y_barrel = Book1D("global_y_barrel", "PixelCluster_global_y_barrel;Global y [mm];Entries;", 64, -350, 350, false);
    m_global_y_endcap = Book1D("global_y_endcap", "PixelCluster_global_y_endcap;Global y [mm];Entries;", 64, -350, 350, false);

    m_global_z_barrel = Book1D("global_z_barrel", "PixelCluster_global_z_barrel;Global z [mm];Entries;", 100, -3000, 3000, false);
    m_global_z_endcap = Book1D("global_z_endcap", "PixelCluster_global_z_endcap;Global z [mm];Entries;", 100, -3000, 3000, false);
    
    m_local_x_barrel = Book1D("local_x_barrel", "PixelCluster_local_x_barrel;Local x [mm];Entries;", 56, -28, 28, false);
    m_local_x_endcap = Book1D("local_x_endcap", "PixelCluster_local_x_endcap;Local x [mm];Entries;", 56, -28, 28, false);

    m_local_y_barrel = Book1D("local_y_barrel", "PixelCluster_local_y_barrel;Local y [mm];Entries;", 56, -28, 28, false);
    m_local_y_endcap = Book1D("local_y_endcap", "PixelCluster_local_y_endcap;Local y [mm];Entries;", 56, -28, 28, false);

    m_localCovXX_barrel = Book1D("localCovXX_barrel", "PixelCluster_localCovXX_barrel;Local Cov XX [mm2];Entries;", 100, 0, 0.9*1e-3, false);
    m_localCovXX_endcap = Book1D("localCovXX_endcap", "PixelCluster_localCovXX_endcap;Local Cov XX [mm2];Entries;", 100, 0, 0.9*1e-3, false);

    m_localCovYY_barrel = Book1D("localCovYY_barrel", "PixelCluster_localCovYY_barrel;Local Cov YY [mm2];Entries;", 100, 0, 0.9*1e-3, false);
    m_localCovYY_endcap = Book1D("localCovYY_endcap", "PixelCluster_localCovYY_endcap;Local Cov YY [mm2];Entries;", 100, 0, 0.9*1e-3, false);

    m_sizeX_barrel = Book1D("sizeX_barrel", "PixelCluster_sizeX_barrel;Size X;Entries;", 100, 0, 400, false);
    m_sizeX_endcap = Book1D("sizeX_endcap", "PixelCluster_sizeX_endcap;Size X;Entries;", 100, 0, 400, false);

    m_sizeY_barrel = Book1D("sizeY_barrel", "PixelCluster_sizeY_barrel;Size Y;Entries;", 100, 0, 400, false);
    m_sizeY_endcap = Book1D("sizeY_endcap", "PixelCluster_sizeY_endcap;Size Y;Entries;", 100, 0, 400, false);

    m_widthY_barrel = Book1D("widthY_barrel", "PixelCluster_widthY_barrel;Width Y;Entries;", 60, 0, 30, false); 
    m_widthY_endcap = Book1D("widthY_endcap", "PixelCluster_widthY_endcap;Width Y;Entries;", 60, 0, 30, false); 

    m_global_xy_barrel = Book2D("global_xy_barrel", "PixelCluster_global_xy_barrel;x [mm];y [mm];", 64, -320, 320, 64, -350, 350, false);
    m_global_xy_endcap = Book2D("global_xy_endcap", "PixelCluster_global_xy_endcap;x [mm];y [mm];", 64, -320, 320, 64, -350, 350, false);

    m_global_zr_barrel = Book2D("global_zr_barrel", "PixelCluster_global_zr_barrel;z [mm];r [mm];", 100, -3000, 3000, 100, 0, 350, false);
    m_global_zr_endcap = Book2D("global_zr_endcap", "PixelCluster_global_zr_endcap;z [mm];r [mm];", 100, -3000, 3000, 100, 0, 350, false);
  }

  void PixelClusterValidationPlots:: fill(const xAOD::PixelCluster* cluster,
					  float beamSpotWeight,
					  const PixelID* pixelID)
  {
    const Identifier& id = pixelID->wafer_id(cluster->identifierHash());
    int pixBrlEc = pixelID->barrel_ec(id);
    int pixLayerDisk = pixelID->layer_disk(id);


    bool isBarrel = (pixBrlEc == 0);

    m_barrelEndcap->Fill(pixBrlEc, beamSpotWeight);

    const auto& local_position = cluster->template localPosition<2>();
    const auto& local_covariance = cluster->template localCovariance<2>();
    const auto& globalPos = cluster->globalPosition();
    Amg::Vector3D globalPosition(globalPos(0, 0), globalPos(1, 0), globalPos(2, 0));

    if (isBarrel) {
      m_layerDisk_barrel->Fill(pixLayerDisk, beamSpotWeight);
      m_phiModule_barrel->Fill(pixelID->phi_module(id), beamSpotWeight);
      m_etaModule_barrel->Fill(pixelID->eta_module(id), beamSpotWeight);
      m_isInnermost_barrel->Fill(static_cast<int>(pixLayerDisk==0), beamSpotWeight);
      m_isNextToInnermost_barrel->Fill(static_cast<int>(pixLayerDisk==1), beamSpotWeight);

      m_eta_barrel->Fill(globalPosition.eta(), beamSpotWeight);
      m_perp_barrel->Fill(globalPosition.perp(), beamSpotWeight);

      m_total_charge_barrel->Fill(cluster->totalCharge(), beamSpotWeight);
      m_total_tot_barrel->Fill(cluster->totalToT(), beamSpotWeight);
    
      m_global_x_barrel->Fill(globalPos(0, 0), beamSpotWeight);
      m_global_y_barrel->Fill(globalPos(1, 0), beamSpotWeight);
      m_global_z_barrel->Fill(globalPos(2, 0), beamSpotWeight);
      
      m_local_x_barrel->Fill(local_position(0, 0), beamSpotWeight);
      m_local_y_barrel->Fill(local_position(1, 0), beamSpotWeight);

      m_localCovXX_barrel->Fill(local_covariance(0, 0), beamSpotWeight);
      m_localCovYY_barrel->Fill(local_covariance(1, 1), beamSpotWeight);

      m_sizeX_barrel->Fill(cluster->channelsInPhi(), beamSpotWeight);
      m_sizeY_barrel->Fill(cluster->channelsInEta(), beamSpotWeight);

      m_widthY_barrel->Fill(cluster->widthInEta(), beamSpotWeight);

      m_global_xy_barrel->Fill(globalPos(0, 0), globalPos(1, 0), beamSpotWeight);
      m_global_zr_barrel->Fill(globalPos(2, 0), globalPosition.perp(), beamSpotWeight);
    } else {
      m_layerDisk_endcap->Fill(pixLayerDisk, beamSpotWeight);
      m_phiModule_endcap->Fill(pixelID->phi_module(id), beamSpotWeight);
      m_etaModule_endcap->Fill(pixelID->eta_module(id), beamSpotWeight);
      m_isInnermost_endcap->Fill(static_cast<int>(pixLayerDisk==0), beamSpotWeight);
      m_isNextToInnermost_endcap->Fill(static_cast<int>(pixLayerDisk==1 or pixLayerDisk==2), beamSpotWeight);

      m_eta_endcap->Fill(globalPosition.eta(), beamSpotWeight);
      m_perp_endcap->Fill(globalPosition.perp(), beamSpotWeight);

      m_total_charge_endcap->Fill(cluster->totalCharge(), beamSpotWeight);
      m_total_tot_endcap->Fill(cluster->totalToT(), beamSpotWeight);
       
      m_global_x_endcap->Fill(globalPos(0, 0), beamSpotWeight);
      m_global_y_endcap->Fill(globalPos(1, 0), beamSpotWeight);
      m_global_z_endcap->Fill(globalPos(2, 0), beamSpotWeight);
      
      m_local_x_endcap->Fill(local_position(0, 0), beamSpotWeight);
      m_local_y_endcap->Fill(local_position(1, 0), beamSpotWeight);

      m_localCovXX_endcap->Fill(local_covariance(0, 0), beamSpotWeight);
      m_localCovYY_endcap->Fill(local_covariance(1, 1), beamSpotWeight);
      
      m_sizeX_endcap->Fill(cluster->channelsInPhi(), beamSpotWeight);
      m_sizeY_endcap->Fill(cluster->channelsInEta(), beamSpotWeight);

      m_widthY_endcap->Fill(cluster->widthInEta(), beamSpotWeight);

      m_global_xy_endcap->Fill(globalPos(0, 0), globalPos(1, 0), beamSpotWeight);
      m_global_zr_endcap->Fill(globalPos(2, 0), globalPosition.perp(), beamSpotWeight);
    }

  }

}

