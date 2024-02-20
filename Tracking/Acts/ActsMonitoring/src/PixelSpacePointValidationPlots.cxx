/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "src/PixelSpacePointValidationPlots.h"
#include "GeoPrimitives/GeoPrimitives.h"

namespace ActsTrk {

  PixelSpacePointValidationPlots::PixelSpacePointValidationPlots(PlotBase* pParent, 
								 const std::string& sDir)
    : PlotBase(pParent, sDir)
  {
    m_barrelEndcap = Book1D("barrelEndcap", "PixelSpacePoint_barrelEndcap;Barrel-Endcap;Entries;", 5, -2, 3, false);

    m_layerDisk_barrel = Book1D("layerDisk_barrel", "PixelSpacePoint_layerDisk_barrel;Layer Disk;Entries;", 9, 0, 9, false);
    m_layerDisk_endcap = Book1D("layerDisk_endcap", "PixelSpacePoint_layerDisk_endcap;Layer Disk;Entries;", 9, 0, 9, false);

    m_phiModule_barrel = Book1D("phiModule_barrel", "PixelSpacePoint_phiModule_barrel;Phi Module;Entries", 56, 0, 56, false);
    m_phiModule_endcap = Book1D("phiModule_endcap", "PixelSpacePoint_phiModule_endcap;Phi Module;Entries", 56, 0, 56, false);

    m_etaModule_barrel = Book1D("etaModule_barrel", "PixelSpacePoint_etaModule_barrel;Eta Module;Entries;", 46, -18, 28, false);
    m_etaModule_endcap = Book1D("etaModule_endcap", "PixelSpacePoint_etaModule_endcap;Eta Module;Entries;", 46, -18, 28, false);

    m_isInnermost_barrel = Book1D("isInnermost_barrel", "PixelSpacePoint_isInnermost_barrel;In Innermost;Entries;", 2, 0, 2, false);
    m_isInnermost_endcap = Book1D("isInnermost_endcap", "PixelSpacePoint_isInnermost_endcap;In Innermost;Entries;", 2, 0, 2, false);

    m_isNextToInnermost_barrel = Book1D("isNextToInnermost_barrel", "PixelSpacePoint_isNextToInnermost_barrel;Is Next To Innermost;Entries", 2, 0, 2, false);
    m_isNextToInnermost_endcap = Book1D("isNextToInnermost_endcap", "PixelSpacePoint_isNextToInnermost_endcap;Is Next To Innermost;Entries", 2, 0, 2, false);

    m_eta_barrel = Book1D("eta_barrel", "PixelSpacePoint_eta_barrel;eta;Entries;", 50, -5, 5, false);
    m_eta_endcap = Book1D("eta_endcap", "PixelSpacePoint_eta_endcap;eta;Entries;", 50, -5, 5, false);

    m_perp_barrel = Book1D("perp_barrel", "PixelSpacePoint_perp_barrel;r [mm];Entries;", 100, 0, 320, false);
    m_perp_endcap = Book1D("perp_endcap", "PixelSpacePoint_perp_endcap;r [mm];Entries;", 100, 0, 320, false);

    m_global_x_barrel = Book1D("global_x_barrel", "PixelSpacePoint_global_x_barrel;Global x [mm];Entries;", 64, -350, 350, false);
    m_global_x_endcap = Book1D("global_x_endcap", "PixelSpacePoint_global_x_endcap;Global x [mm];Entries;", 64, -350, 350, false);

    m_global_y_barrel = Book1D("global_y_barrel", "PixelSpacePoint_global_y_barrel;Global y [mm];Entries;", 64, -350, 350, false);
    m_global_y_endcap = Book1D("global_y_endcap", "PixelSpacePoint_global_y_endcap;Global y [mm];Entries;", 64, -350, 350, false);

    m_global_z_barrel = Book1D("global_z_barrel", "PixelSpacePoint_global_z_barrel;Global z [mm];Entries;", 100, -3000, 3000, false);
    m_global_z_endcap = Book1D("global_z_endcap", "PixelSpacePoint_global_z_endcap;Global z [mm];Entries;", 100, -3000, 3000, false);
    
    m_globalCovR_barrel = Book1D("globalCovR_barrel", "PixelSpacePoint_globalCovR_barrel;Global Cov R;Entries;", 70, 0, 140, false);
    m_globalCovR_endcap = Book1D("globalCovR_endcap", "PixelSpacePoint_globalCovR_endcap;Global Cov R;Entries;", 70, 0, 140, false);

    m_globalCovZ_barrel = Book1D("globalCovZ_barrel", "PixelSpacePoint_globalCovZ_barrel;Global Cov Z;Entries;", 70, 0, 140, false);
    m_globalCovZ_endcap = Book1D("globalCovZ_endcap", "PixelSpacePoint_globalCovZ_endcap;Global Cov Z;Entries;", 70, 0, 140, false);

    m_global_xy_barrel = Book2D("global_xy_barrel", "PixelSpacePoint_global_xy_barrel;x [mm];y [mm];", 64, -320, 320, 64, -350, 350, false);
    m_global_xy_endcap = Book2D("global_xy_endcap", "PixelSpacePoint_global_xy_endcap;x [mm];y [mm];", 64, -320, 320, 64, -350, 350, false);

    m_global_zr_barrel = Book2D("global_zr_barrel", "PixelSpacePoint_global_zr_barrel;z [mm];r [mm];", 100, -3000, 3000, 100, 0, 350, false);
    m_global_zr_endcap = Book2D("global_zr_endcap", "PixelSpacePoint_global_zr_endcap;z [mm];r [mm];", 100, -3000, 3000, 100, 0, 350, false);
  }

  void PixelSpacePointValidationPlots::fill(const xAOD::SpacePoint* spacePoint,
					    float beamSpotWeight,
					    const PixelID* pixelID)
  {
    const Identifier& id = pixelID->wafer_id(spacePoint->elementIdList()[0]);
    bool isBarrel = (pixelID->barrel_ec(id) == 0);
    int pixLayerDisk = pixelID->layer_disk(id);

    const auto& globalPos = spacePoint->globalPosition();
    Amg::Vector3D globalPosition(globalPos(0, 0), globalPos(1, 0), globalPos(2, 0));

    m_barrelEndcap->Fill(pixelID->barrel_ec(id), beamSpotWeight);

    if (isBarrel) {
      m_layerDisk_barrel->Fill(pixLayerDisk, beamSpotWeight);
      m_phiModule_barrel->Fill(pixelID->phi_module(id), beamSpotWeight);
      m_etaModule_barrel->Fill(pixelID->eta_module(id), beamSpotWeight);

      m_isInnermost_barrel->Fill(static_cast<int>(pixLayerDisk==0), beamSpotWeight);
      m_isNextToInnermost_barrel->Fill(static_cast<int>(pixLayerDisk==1), beamSpotWeight);

      m_eta_barrel->Fill(globalPosition.eta(), beamSpotWeight);
      m_perp_barrel->Fill(globalPosition.perp() , beamSpotWeight);

      m_global_x_barrel->Fill(globalPos(0, 0), beamSpotWeight);
      m_global_y_barrel->Fill(globalPos(1, 0), beamSpotWeight);
      m_global_z_barrel->Fill(globalPos(2, 0), beamSpotWeight);

      m_globalCovR_barrel->Fill(spacePoint->varianceR(), beamSpotWeight);
      m_globalCovZ_barrel->Fill(spacePoint->varianceZ(), beamSpotWeight);

      m_global_xy_barrel->Fill(globalPos(0, 0), globalPos(1, 0), beamSpotWeight);
      m_global_zr_barrel->Fill(globalPos(2, 0), globalPosition.perp(), beamSpotWeight);
    } else {
      m_layerDisk_endcap->Fill(pixLayerDisk, beamSpotWeight);
      m_phiModule_endcap->Fill(pixelID->phi_module(id), beamSpotWeight);
      m_etaModule_endcap->Fill(pixelID->eta_module(id), beamSpotWeight);

      m_isInnermost_endcap->Fill(static_cast<int>(pixLayerDisk==0), beamSpotWeight);
      m_isNextToInnermost_endcap->Fill(static_cast<int>(pixLayerDisk==1), beamSpotWeight);

      m_eta_endcap->Fill(globalPosition.eta(), beamSpotWeight);
      m_perp_endcap->Fill(globalPosition.perp() , beamSpotWeight);

      m_global_x_endcap->Fill(globalPos(0, 0), beamSpotWeight);
      m_global_y_endcap->Fill(globalPos(1, 0), beamSpotWeight);
      m_global_z_endcap->Fill(globalPos(2, 0), beamSpotWeight);

      m_globalCovR_endcap->Fill(spacePoint->varianceR(), beamSpotWeight);
      m_globalCovZ_endcap->Fill(spacePoint->varianceZ(), beamSpotWeight);

      m_global_xy_endcap->Fill(globalPos(0, 0), globalPos(1, 0), beamSpotWeight);
      m_global_zr_endcap->Fill(globalPos(2, 0), globalPosition.perp(), beamSpotWeight);
    }
  }

}
