/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTS_HGTD_CLUSTER_VALIDATION_PLOTS_H
#define ACTS_HGTD_CLUSTER_VALIDATION_PLOTS_H

#include "TrkValHistUtils/PlotBase.h" 
#include "xAODInDetMeasurement/HGTDClusterContainer.h"
#include "HGTD_Identifier/HGTD_ID.h"
#include "HGTD_ReadoutGeometry/HGTD_DetectorElementCollection.h"

namespace ActsTrk {

  class HgtdClusterValidationPlots :
    public PlotBase {
  public:
    HgtdClusterValidationPlots(PlotBase* pParent, const std::string& sDir);
    virtual ~HgtdClusterValidationPlots() = default;

    void fill(const xAOD::HGTDCluster* cluster, 
	      const InDetDD::HGTD_DetectorElementCollection& hgtdElements,
	      float beamSpotWeight,
	      const HGTD_ID*);
    
  private:
    TH1* m_barrelEndcap {};

    TH1* m_layer_right {};
    TH1* m_layer_left {};

    TH1* m_phi_module_right {};
    TH1* m_phi_module_left {};

    TH1* m_eta_module_right {};
    TH1* m_eta_module_left {};

    TH1* m_phi_index_right {};
    TH1* m_phi_index_left {};

    TH1* m_eta_index_right {};
    TH1* m_eta_index_left {};

    TH1* m_eta {};
    
    TH1* m_local_x_right {};
    TH1* m_local_y_right {};
    TH1* m_local_t_right {};

    TH1* m_localCovXX_right {};
    TH1* m_localCovYY_right {};
    TH1* m_localCovTT_right {};

    TH2* m_local_xy_right {};
    TH2* m_global_xy_right {};
    TH2* m_global_zr_right {};

    TH1* m_local_x_left {};
    TH1* m_local_y_left {};
    TH1* m_local_t_left {};

    TH1* m_localCovXX_left {};
    TH1* m_localCovYY_left {};
    TH1* m_localCovTT_left {};

    TH1* m_global_x_left {};
    TH1* m_global_y_left {};
    TH1* m_global_z_left {};
    TH1* m_global_r_left {};

    TH1* m_global_x_right {};
    TH1* m_global_y_right {};
    TH1* m_global_z_right {};
    TH1* m_global_r_right {};
    
    TH2* m_local_xy_left {};
    TH2* m_global_xy_left {};
    TH2* m_global_zr_left {};
  };

}

#endif
