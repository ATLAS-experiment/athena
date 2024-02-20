/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTS_PIXEL_SPACE_POINT_VALIDATION_PLOTS_H
#define ACTS_PIXEL_SPACE_POINT_VALIDATION_PLOTS_H

#include "TrkValHistUtils/PlotBase.h" 
#include "xAODInDetMeasurement/SpacePointContainer.h"
#include "InDetIdentifier/PixelID.h"

namespace ActsTrk {

  class PixelSpacePointValidationPlots :
    public PlotBase {
  public:
    PixelSpacePointValidationPlots(PlotBase* pParent, const std::string& sDir);
    virtual ~PixelSpacePointValidationPlots() = default;

    void fill(const xAOD::SpacePoint* spacePoint, 
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

    TH1* m_global_x_barrel {};
    TH1* m_global_x_endcap {};

    TH1* m_global_y_barrel {};
    TH1* m_global_y_endcap {};

    TH1* m_global_z_barrel {};
    TH1* m_global_z_endcap {};

    TH1* m_globalCovR_barrel {};
    TH1* m_globalCovR_endcap {};

    TH1* m_globalCovZ_barrel {};
    TH1* m_globalCovZ_endcap {};

    TH2* m_global_xy_barrel {};
    TH2* m_global_xy_endcap {};

    TH2* m_global_zr_barrel {};
    TH2* m_global_zr_endcap {};
  };

}

#endif
