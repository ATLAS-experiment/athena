/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTS_STRIP_CLUSTER_VALIDATION_PLOTS_H
#define ACTS_STRIP_CLUSTER_VALIDATION_PLOTS_H

#include "TrkValHistUtils/PlotBase.h" 
#include "xAODInDetMeasurement/StripClusterContainer.h"
#include "InDetIdentifier/SCT_ID.h"

namespace ActsTrk {

  class StripClusterValidationPlots :
    public PlotBase {
  public:
    StripClusterValidationPlots(PlotBase* pParent, const std::string& sDir);
    virtual ~StripClusterValidationPlots() = default;

    void fill(const xAOD::StripCluster* cluster, 
	      float beamSpotWeight,
	      const SCT_ID*);
    
  private:
    TH1* m_barrelEndcap {};

    TH1* m_layerDisk_barrel {};
    TH1* m_layerDisk_endcap {};

    TH1* m_phiModule_barrel {};
    TH1* m_phiModule_endcap {};

    TH1* m_etaModule_barrel {};
    TH1* m_etaModule_endcap {};

    TH1* m_sideModule_barrel {};
    TH1* m_sideModule_endcap {};

    TH1* m_eta_barrel {};
    TH1* m_eta_endcap {};

    TH1* m_perp_barrel {};
    TH1* m_perp_endcap {};

    TH1* m_global_x_barrel {};
    TH1* m_global_x_endcap {};

    TH1* m_global_y_barrel {};
    TH1* m_global_y_endcap {};

    TH1* m_global_z_barrel {};
    TH1* m_global_z_endcap {};

    TH1* m_local_x_barrel {};
    TH1* m_local_x_endcap {};

    TH1* m_localCovXX_barrel {};
    TH1* m_localCovXX_endcap {};

    TH1* m_sizeX_barrel {};
    TH1* m_sizeX_endcap {};

    TH2* m_global_xy_barrel {};
    TH2* m_global_xy_endcap {};

    TH2* m_global_zr_barrel {};
    TH2* m_global_zr_endcap {};
  };

}

#endif
