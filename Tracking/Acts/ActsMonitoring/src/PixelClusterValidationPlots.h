/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTS_PIXEL_CLUSTER_VALIDATION_PLOTS_H
#define ACTS_PIXEL_CLUSTER_VALIDATION_PLOTS_H

#include "TrkValHistUtils/PlotBase.h" 
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "InDetIdentifier/PixelID.h"

namespace ActsTrk {

  class PixelClusterValidationPlots :
    public PlotBase {
  public:
    PixelClusterValidationPlots(PlotBase* pParent, const std::string& sDir);
    virtual ~PixelClusterValidationPlots() = default;

    void fill(const xAOD::PixelCluster* cluster, 
	      float beamSpotWeight,
	      const PixelID*);
    
  private:
    TH1* m_barrelEndcap {};

    TH1* m_layerDisk_barrel {};
    TH1* m_layerDisk_endcap {};

    TH1* m_phiModule_barrel {};
    TH1* m_phiModule_endcap {};

    TH1* m_etaModule_barrel {};
    TH1* m_etaModule_endcap {};

    TH1* m_isInnermost_barrel {};    
    TH1* m_isInnermost_endcap {};

    TH1* m_isNextToInnermost_barrel {};
    TH1* m_isNextToInnermost_endcap {};

    TH1* m_eta_barrel {};
    TH1* m_eta_endcap {};

    TH1* m_perp_barrel {};
    TH1* m_perp_endcap {};

    TH1* m_total_charge_barrel {};
    TH1* m_total_charge_endcap {};

    TH1* m_total_tot_barrel {};
    TH1* m_total_tot_endcap {};

    TH1* m_omega_x_barrel {};
    TH1* m_omega_x_endcap {};

    TH1* m_omega_y_barrel {};
    TH1* m_omega_y_endcap {};

    TH1* m_lvl1a_barrel {};
    TH1* m_lvl1a_endcap {};
    
    TH1* m_global_x_barrel {};
    TH1* m_global_x_endcap {};

    TH1* m_global_y_barrel {};
    TH1* m_global_y_endcap {};

    TH1* m_global_z_barrel {};
    TH1* m_global_z_endcap {};

    TH1* m_local_x_barrel {};
    TH1* m_local_x_endcap {};

    TH1* m_local_y_barrel {};
    TH1* m_local_y_endcap {};

    TH1* m_localCovXX_barrel {};
    TH1* m_localCovXX_endcap {};

    TH1* m_localCovYY_barrel {};
    TH1* m_localCovYY_endcap {};

    TH1* m_sizeX_barrel {};
    TH1* m_sizeX_endcap {};

    TH1* m_sizeY_barrel {};
    TH1* m_sizeY_endcap {};

    TH1* m_widthY_barrel {};
    TH1* m_widthY_endcap {};

    TH1* m_charge_barrel {};
    TH1* m_charge_endcap {};
    
    TH1* m_tot_barrel {};
    TH1* m_tot_endcap {};

    TH2* m_charge_vs_tot_barrel {};
    TH2* m_charge_vs_tot_endcap {};
    
    TH2* m_global_xy_barrel {};
    TH2* m_global_xy_endcap {};

    TH2* m_global_zr_barrel {};
    TH2* m_global_zr_endcap {};

    TH2* m_sizeX_vs_eta_barrel {};
    TH2* m_sizeX_vs_eta_endcap {};

    TH2* m_sizeY_vs_eta_barrel {};
    TH2* m_sizeY_vs_eta_endcap {};

    TH2* m_sizeX_vs_sizeY_barrel {};
    TH2* m_sizeX_vs_sizeY_endcap {};
  };

}

#endif
