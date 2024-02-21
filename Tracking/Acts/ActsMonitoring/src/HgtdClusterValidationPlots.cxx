/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "src/HgtdClusterValidationPlots.h"
#include "HGTD_ReadoutGeometry/HGTD_DetectorElement.h"
#include "TrkSurfaces/Surface.h"

namespace ActsTrk {

  HgtdClusterValidationPlots::HgtdClusterValidationPlots(PlotBase* pParent, 
							 const std::string& sDir)
    : PlotBase(pParent, sDir)
  {
    m_barrelEndcap = Book1D("barrelEndcap", "HgtdCluster_barrelEndcap;Barrel-Endcap;Entries;", 5, -2, 3, false);

    m_layer_right = Book1D("layer_right", "HgtdCluster_layer_right;Layer;Entries;", 20, -10, 11, false);
    m_layer_left = Book1D("layer_left", "HgtdCluster_layer_left;Layer;Entries;", 20, -10, 11, false);

    m_phi_module_right = Book1D("phi_module_right", "HgtdCluster_phi_module_right;Phi Module;Entries;", 100, 0, 300, false);
    m_phi_module_left = Book1D("phi_module_left", "HgtdCluster_phi_module_left;Phi Module;Entries;", 100, 0, 300, false);

    m_eta_module_right = Book1D("eta_module_right", "HgtdCluster_eta_module_right;Eta Module;Entries;", 100, -10, 10, false);
    m_eta_module_left = Book1D("eta_module_left", "HgtdCluster_eta_module_left;Eta Module;Entries;", 100, -10, 10, false);

    m_phi_index_right = Book1D("phi_index_right", "HgtdCluster_phi_index_right;Phi Index;Entries;", 100, -10, 10, false);
    m_phi_index_left = Book1D("phi_index_left", "HgtdCluster_phi_index_left;Phi Index;Entries;", 100, -10, 10, false);

    m_eta_index_right = Book1D("eta_index_right", "HgtdCluster_eta_index_right;Eta Index;Entries;", 100, -10, 10, false);
    m_eta_index_left = Book1D("eta_index_left", "HgtdCluster_eta_index_left;Eta Index;Entries;", 100, -10, 10, false);

    m_local_x_right = Book1D("local_x_right", "HgtdCluster_local_x_right;Local x [mm];Entries;", 100, -30, 30, false);
    m_local_y_right = Book1D("local_y_right", "HgtdCluster_local_y_right;Local y [mm];Entries;", 100, -30, 30, false);
    m_local_t_right = Book1D("local_t_right", "HgtdCluster_local_t_right;Local t [ns];Entries;", 30, 0, 50, false);

    m_localCovXX_right = Book1D("localCovXX_right", "HgtdCluster_localCovXX_right;Local Cov XX [mm2];Entries;", 100, 0, 0.5, false);
    m_localCovYY_right = Book1D("localCovYY_right", "HgtdCluster_localCovYY_right;Local Cov YY [mm2];Entries;", 100, 0, 0.5, false);
    m_localCovTT_right = Book1D("localCovTT_right", "HgtdCluster_localCovTT_right;Local Cov TT [ns2];Entries;", 100, 0, 0.2, false);

    m_local_xy_right = Book2D("local_xy_right", "HgtdCluster_local_xy_right;Local x [mm];Local y [mm];", 100, -30, 30, 100, -30, 30, false);

    m_local_x_left = Book1D("local_x_left", "HgtdCluster_local_x_left;Local x [mm];Entries;", 100, -30, 30, false);
    m_local_y_left = Book1D("local_y_left", "HgtdCluster_local_y_left;Local y [mm];Entries;", 100, -30, 30, false);
    m_local_t_left = Book1D("local_t_left", "HgtdCluster_local_t_left;Local t [ns];Entries;", 30, 0, 50, false);

    m_localCovXX_left = Book1D("localCovXX_left", "HgtdCluster_localCovXX_left;Local Cov XX [mm2];Entries;", 100, 0, 0.5, false);
    m_localCovYY_left = Book1D("localCovYY_left", "HgtdCluster_localCovYY_left;Local Cov YY [mm2];Entries;", 100, 0, 0.5, false);
    m_localCovTT_left = Book1D("localCovTT_left", "HgtdCluster_localCovTT_left;Local Cov TT [ns2];Entries;", 100, 0, 0.2, false);

    m_eta = Book1D("eta", "HgtdCluster_eta;Eta;Entries;", 100, -5, 5, false);
    
    m_global_x_left = Book1D("global_x_left", "HgtdCluster_global_x_left;Global x [mm];Entries;", 100, -1100, 1100, false);
    m_global_x_right = Book1D("global_x_right", "HgtdCluster_global_x_right;Global x [mm];Entries;", 100, -1100, 1100, false);

    m_global_y_left = Book1D("global_y_left", "HgtdCluster_global_y_left;Global y [mm];Entries;", 100, -1100, 1100, false);
    m_global_y_right = Book1D("global_y_right", "HgtdCluster_global_y_right;Global y [mm];Entries;", 100, -1100, 1100, false);

    m_global_z_left = Book1D("global_z_left", "HgtdCluster_global_z_left;Global z [mm];Entries;", 100, -4000, -3000, false);
    m_global_z_right = Book1D("global_z_right", "HgtdCluster_global_z_right;Global z [mm];Entries;", 100, 3000, 4000, false);

    m_global_r_left = Book1D("global_r_left", "HgtdCluster_global_r_left;Global r [mm];Entries;", 100, 0, 900, false);
    m_global_r_right = Book1D("global_r_right", "HgtdCluster_global_r_right;Global r [mm];Entries;", 100, 0, 900, false);
    
    m_local_xy_left = Book2D("local_xy_left", "HgtdCluster_local_xy_left;Local x [mm];Local y [mm];", 100, -30, 30, 100, -30, 30, false);

    m_global_xy_left = Book2D("global_xy_left", "HgtdCluster_global_xy_left;Global x [mm];Global y [mm];", 100, -1100, 1100, 100, -1100, 1100, false);
    m_global_xy_right = Book2D("global_xy_right", "HgtdCluster_global_xy_right;Global x [mm];global y [mm];", 100, -1100, 1100, 100, -1100, 1100, false);

    m_global_zr_left = Book2D("global_zr_left", "HgtdCluster_global_zr_left;Global z [mm];Global r [mm];", 100, -4000, -3000, 100, 0, 900, false);
    m_global_zr_right = Book2D("global_zr_right", "HgtdCluster_global_zr_right;Global z [mm];Global r [mm];", 100, 3000, 4000, 100, 0, 900, false);
}

  void HgtdClusterValidationPlots::fill(const xAOD::HGTDCluster* cluster,
					const InDetDD::HGTD_DetectorElementCollection& hgtdElements,
					float beamSpotWeight,
					const HGTD_ID* hgtdID)
  {
    const Identifier& id = hgtdID->wafer_id(cluster->identifierHash());
    const auto *element = hgtdElements.getDetectorElement(hgtdID->wafer_hash(hgtdID->wafer_id(id)));
    
    const auto& local_position = cluster->template localPosition<3>();
    const auto& local_covariance = cluster->template localCovariance<3>();

    // compute global position
    const Amg::Transform3D& T = element->surface().transform();
    double Ax[3] = {T(0,0),T(1,0),T(2,0)};
    double Ay[3] = {T(0,1),T(1,1),T(2,1)};
    double R [3] = {T(0,3),T(1,3),T(2,3)};
    
    Amg::Vector2D M;
    M[0] = local_position(0,0);
    M[1] = local_position(1,0);
    Amg::Vector3D globalPos(M[0]*Ax[0]+M[1]*Ay[0]+R[0],M[0]*Ax[1]+M[1]*Ay[1]+R[1],M[0]*Ax[2]+M[1]*Ay[2]+R[2]);
    
    m_barrelEndcap->Fill(hgtdID->endcap(id));
    m_eta->Fill(globalPos.eta(), beamSpotWeight);
      
    // Divide in left and right endcaps
    if (hgtdID->endcap(id) == -2) {
      m_layer_left->Fill(hgtdID->layer(id), beamSpotWeight);
      m_phi_module_left->Fill(hgtdID->phi_module(id), beamSpotWeight);
      m_eta_module_left->Fill(hgtdID->eta_module(id), beamSpotWeight);
      m_phi_index_left->Fill(hgtdID->phi_index(id), beamSpotWeight);
      m_eta_index_left->Fill(hgtdID->eta_index(id), beamSpotWeight);

      m_local_x_left->Fill(local_position(0, 0), beamSpotWeight);
      m_local_y_left->Fill(local_position(1, 0), beamSpotWeight);
      m_local_t_left->Fill(local_position(2, 0), beamSpotWeight);
      
      m_localCovXX_left->Fill(local_covariance(0, 0), beamSpotWeight);
      m_localCovYY_left->Fill(local_covariance(1, 1), beamSpotWeight);
      m_localCovTT_left->Fill(local_covariance(2, 2), beamSpotWeight);

      m_global_x_left->Fill(globalPos.x(), beamSpotWeight);
      m_global_y_left->Fill(globalPos.y(), beamSpotWeight);
      m_global_z_left->Fill(globalPos.z(), beamSpotWeight);
      m_global_r_left->Fill(std::sqrt(globalPos.x()*globalPos.x() + (globalPos.y()*globalPos.y())), beamSpotWeight);
      
      m_local_xy_left->Fill(local_position(0, 0), local_position(1, 0), beamSpotWeight);
      m_global_xy_left->Fill(globalPos.x(), globalPos.y(), beamSpotWeight);
      m_global_zr_left->Fill(globalPos.z(), std::sqrt(globalPos.x()*globalPos.x() + (globalPos.y()*globalPos.y())), beamSpotWeight);	
    } else if (hgtdID->endcap(id) == 2) {
      m_layer_right->Fill(hgtdID->layer(id), beamSpotWeight);
      m_phi_module_right->Fill(hgtdID->phi_module(id), beamSpotWeight);
      m_eta_module_right->Fill(hgtdID->eta_module(id), beamSpotWeight);
      m_phi_index_right->Fill(hgtdID->phi_index(id), beamSpotWeight);
      m_eta_index_right->Fill(hgtdID->eta_index(id), beamSpotWeight);

      m_local_x_right->Fill(local_position(0, 0), beamSpotWeight);
      m_local_y_right->Fill(local_position(1, 0), beamSpotWeight);
      m_local_t_right->Fill(local_position(2, 0), beamSpotWeight);
      
      m_localCovXX_right->Fill(local_covariance(0, 0), beamSpotWeight);
      m_localCovYY_right->Fill(local_covariance(1, 1), beamSpotWeight);
      m_localCovTT_right->Fill(local_covariance(2, 2), beamSpotWeight);

      m_global_x_right->Fill(globalPos.x(), beamSpotWeight);
      m_global_y_right->Fill(globalPos.y(), beamSpotWeight);
      m_global_z_right->Fill(globalPos.z(), beamSpotWeight);
      m_global_r_right->Fill(std::sqrt(globalPos.x()*globalPos.x() + (globalPos.y()*globalPos.y())), beamSpotWeight);

      m_local_xy_right->Fill(local_position(0, 0), local_position(1, 0), beamSpotWeight);
      m_global_xy_right->Fill(globalPos.x(), globalPos.y(), beamSpotWeight);
      m_global_zr_right->Fill(globalPos.z(), std::sqrt(globalPos.x()*globalPos.x() + (globalPos.y()*globalPos.y())), beamSpotWeight);
    }
    
  }
}

