/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "src/StripSpacePointValidationPlots.h"
#include "GeoPrimitives/GeoPrimitives.h"

namespace ActsTrk {

  StripSpacePointValidationPlots::StripSpacePointValidationPlots(PlotBase* pParent, 
								 const std::string& sDir,
								 const std::string& type)
    : PlotBase(pParent, sDir),
      m_type(type)
  {
    m_barrelEndcap = Book1D("barrelEndcap", m_type + "SpacePoint_barrelEndcap;Barrel-Endcap;Entries;", 5, -2, 3, false);

    m_layerDisk_barrel = Book1D("layerDisk_barrel", m_type + "SpacePoint_layerDisk_barrel;Layer Disk;Entries;", 6, 0, 6, false);
    m_layerDisk_endcap = Book1D("layerDisk_endcap", m_type + "SpacePoint_layerDisk_endcap;Layer Disk;Entries;", 6, 0, 6, false);

    m_phiModule_barrel = Book1D("phiModule_barrel", m_type + "SpacePoint_phiModule_barrel;Phi Module;Entries", 75, 0, 75, false);
    m_phiModule_endcap = Book1D("phiModule_endcap", m_type + "SpacePoint_phiModule_endcap;Phi Module;Entries", 75, 0, 75, false);

    m_etaModule_barrel = Book1D("etaModule_barrel", m_type + "SpacePoint_etaModule_barrel;Eta Module;Entries;", 120, -60, 60, false);
    m_etaModule_endcap = Book1D("etaModule_endcap", m_type + "SpacePoint_etaModule_endcap;Eta Module;Entries;", 120, -60, 60, false);

    m_sideModule_barrel = Book1D("sideModule_barrel", m_type + "SpacePoint_sideModule_barrel;Side Module;Entries;", 2, 0, 2, false);
    m_sideModule_endcap = Book1D("sideModule_endcap", m_type + "SpacePoint_sideModule_endcap;Side Module;Entries;", 2, 0, 2, false);

    m_eta_barrel = Book1D("eta_barrel", m_type + "SpacePoint_eta_barrel;eta;Entries;", 30, -3, 3, false);
    m_eta_endcap = Book1D("eta_endcap", m_type + "SpacePoint_eta_endcap;eta;Entries;", 30, -3, 3, false);

    m_perp_barrel = Book1D("perp_barrel", m_type + "SpacePoint_perp_barrel;r [mm];Entries;", 100, 300, 1100, false);
    m_perp_endcap = Book1D("perp_endcap", m_type + "SpacePoint_perp_endcap;r [mm];Entries;", 100, 300, 1100, false);

    m_global_x_barrel = Book1D("global_x_barrel", m_type + "SpacePoint_global_x_barrel;Global x [mm];Entries;", 100, -1100, 1100, false);
    m_global_x_endcap = Book1D("global_x_endcap", m_type + "SpacePoint_global_x_endcap;Global x [mm];Entries;", 100, -1100, 1100, false);

    m_global_y_barrel = Book1D("global_y_barrel", m_type + "SpacePoint_global_y_barrel;Global y [mm];Entries;", 100, -1100, 1100, false);
    m_global_y_endcap = Book1D("global_y_endcap", m_type + "SpacePoint_global_y_endcap;Global y [mm];Entries;", 100, -1100, 1100, false);

    m_global_z_barrel = Book1D("global_z_barrel", m_type + "SpacePoint_global_z_barrel;Global z [mm];Entries;", 100, -3000, 3000, false);
    m_global_z_endcap = Book1D("global_z_endcap", m_type + "SpacePoint_global_z_endcap;Global z [mm];Entries;", 100, -3000, 3000, false);
    
    m_globalCovR_barrel = Book1D("globalCovR_barrel", m_type + "SpacePoint_globalCovR_barrel; Global Cov R;Entries;", 50, 0, 6, false);
    m_globalCovR_endcap = Book1D("globalCovR_endcap", m_type + "SpacePoint_globalCovR_endcap; Global Cov R;Entries;", 50, 0, 6, false);

    m_globalCovZ_barrel = Book1D("globalCovZ_barrel", m_type + "SpacePoint_globalCovZ_barrel; Global Cov Z;Entries;", 50, 0, 6, false);
    m_globalCovZ_endcap = Book1D("globalCovZ_endcap", m_type + "SpacePoint_globalCovZ_endcap; Global Cov Z;Entries;", 50, 0, 6, false);

    m_topHalfStripLength_barrel = Book1D("topHalfStripLength_barrel", m_type + "SpacePoint_topHalfStripLength_barrel;lenght;Entries;", 100, 0, 35, false);
    m_topHalfStripLength_endcap = Book1D("topHalfStripLength_endcap", m_type + "SpacePoint_topHalfStripLength_endcap;lenght;Entries;", 100, 0, 35, false);

    m_bottomHalfStripLength_barrel = Book1D("bottomHalfStripLength_barrel", m_type + "SpacePoint_bottomHalfStripLength_barrel;length;Entries;", 100, 0, 35, false);
    m_bottomHalfStripLength_endcap = Book1D("bottomHalfStripLength_endcap", m_type + "SpacePoint_bottomHalfStripLength_endcap;length;Entries;", 100, 0, 35, false);

    m_topStripDirection_barrel = Book1D("topStripDirection_barrel", m_type + "SpacePoint_topStripDirection_barrel;;Entries;", 100, -1, 1, false);
    m_topStripDirection_endcap = Book1D("topStripDirection_endcap", m_type + "SpacePoint_topStripDirection_endcap;;Entries;", 100, -1, 1, false);

    m_bottomStripDirection_barrel = Book1D("bottomStripDirection_barrel", m_type + "SpacePoint_bottomStripDirection_barrel;;Entries;", 100, -1, 1, false);
    m_bottomStripDirection_endcap = Book1D("bottomStripDirection_endcap", m_type + "SpacePoint_bottomStripDirection_endcap;;Entries;", 100, -1, 1, false);

    m_stripCenterDistance_barrel = Book1D("stripCenterDistance_barrel", m_type + "SpacePoint_stripCenterDistance_barrel;;Entries;", 100, -60, 60, false);
    m_stripCenterDistance_endcap = Book1D("stripCenterDistance_endcap", m_type + "SpacePoint_stripCenterDistance_endcap;;Entries;", 100, -60, 60, false);

    m_topStripCenter_barrel = Book1D("topStripCenter_barrel", m_type + "SpacePoint_topStripCenter_barrel;;Entries;", 400, -3000, 3000, false);
    m_topStripCenter_endcap = Book1D("topStripCenter_endcap", m_type + "SpacePoint_topStripCenter_endcap;;Entries;", 400, -3000, 3000, false);

    m_global_xy_barrel = Book2D("global_xy_barrel", m_type + "SpacePoint_global_xy_barrel;x [mm];y [mm];", 100, -1100, 1100, 100, -1100, 1100, false);
    m_global_xy_endcap = Book2D("global_xy_endcap", m_type + "SpacePoint_global_xy_endcap;x [mm];y [mm];", 100, -1100, 1100, 100, -1100, 1100, false);

    m_global_zr_barrel = Book2D("global_zr_barrel", m_type + "SpacePoint_global_zr_barrel;z [mm];r [mm];", 100, -3000, 3000, 100, 300, 1100, false);
    m_global_zr_endcap = Book2D("global_zr_endcap", m_type + "SpacePoint_global_zr_endcap;z [mm];r [mm];", 100, -3000, 3000, 100, 300, 1100, false);
  }

  void StripSpacePointValidationPlots::fill(const xAOD::SpacePoint* spacePoint,
					    float beamSpotWeight,
					    const SCT_ID* stripID)
  {
    const std::vector<Identifier>& ids { stripID->wafer_id(spacePoint->elementIdList()[0]), 
	stripID->wafer_id(spacePoint->elementIdList()[1])};

    const auto& globalPos = spacePoint->globalPosition();
    Amg::Vector3D globalPosition(globalPos(0, 0), globalPos(1, 0), globalPos(2, 0));

    const auto& topStripDirection = spacePoint->topStripDirection();
    const auto& bottomStripDirection = spacePoint->bottomStripDirection();
    const auto& stripCenterDistance = spacePoint->stripCenterDistance();
    const auto& topStripCenter = spacePoint->topStripCenter();
    
    for (const Identifier& id : ids) {
      bool isBarrel = (stripID->barrel_ec(id) == 0);

      m_barrelEndcap->Fill(stripID->barrel_ec(id), beamSpotWeight);
      
      if (isBarrel) {
	m_layerDisk_barrel->Fill(stripID->layer_disk(id), beamSpotWeight);
	m_phiModule_barrel->Fill(stripID->phi_module(id), beamSpotWeight);
	m_etaModule_barrel->Fill(stripID->eta_module(id), beamSpotWeight);
	m_sideModule_barrel->Fill(stripID->side(id), beamSpotWeight);
      } else {
	m_layerDisk_endcap->Fill(stripID->layer_disk(id), beamSpotWeight);
	m_phiModule_endcap->Fill(stripID->phi_module(id), beamSpotWeight);
	m_etaModule_endcap->Fill(stripID->eta_module(id), beamSpotWeight);
	m_sideModule_endcap->Fill(stripID->side(id), beamSpotWeight);
      }
    } // loops on ids 

    bool isBarrel = (stripID->barrel_ec(ids[0]) == 0);    

    if (isBarrel) {	
      m_eta_barrel->Fill(globalPosition.eta(), beamSpotWeight);
      m_perp_barrel->Fill(globalPosition.perp() , beamSpotWeight);
      
      m_global_x_barrel->Fill(globalPos(0, 0), beamSpotWeight);
      m_global_y_barrel->Fill(globalPos(1, 0), beamSpotWeight);
      m_global_z_barrel->Fill(globalPos(2, 0), beamSpotWeight);
      
      m_globalCovR_barrel->Fill(spacePoint->varianceR(), beamSpotWeight);
      m_globalCovZ_barrel->Fill(spacePoint->varianceZ(), beamSpotWeight);
      
      m_global_xy_barrel->Fill(globalPos(0, 0), globalPos(1, 0), beamSpotWeight);
      m_global_zr_barrel->Fill(globalPos(2, 0), globalPosition.perp(), beamSpotWeight);
      
      m_topHalfStripLength_barrel->Fill(spacePoint->topHalfStripLength(), beamSpotWeight);
      m_bottomHalfStripLength_barrel->Fill(spacePoint->bottomHalfStripLength(), beamSpotWeight);

      for(int i(0); i<3; ++i) {
	m_topStripDirection_barrel->Fill(topStripDirection(i,0), beamSpotWeight);
	m_bottomStripDirection_barrel->Fill(bottomStripDirection(i,0), beamSpotWeight);
	m_stripCenterDistance_barrel->Fill(stripCenterDistance(i,0), beamSpotWeight);
	m_topStripCenter_barrel->Fill(topStripCenter(i,0), beamSpotWeight);
      }
      
    } else {	
      m_eta_endcap->Fill(globalPosition.eta(), beamSpotWeight);
      m_perp_endcap->Fill(globalPosition.perp() , beamSpotWeight);
      
      m_global_x_endcap->Fill(globalPos(0, 0), beamSpotWeight);
      m_global_y_endcap->Fill(globalPos(1, 0), beamSpotWeight);
      m_global_z_endcap->Fill(globalPos(2, 0), beamSpotWeight);
      
      m_globalCovR_endcap->Fill(spacePoint->varianceR(), beamSpotWeight);
      m_globalCovZ_endcap->Fill(spacePoint->varianceZ(), beamSpotWeight);
      
      m_global_xy_endcap->Fill(globalPos(0, 0), globalPos(1, 0), beamSpotWeight);
      m_global_zr_endcap->Fill(globalPos(2, 0), globalPosition.perp(), beamSpotWeight);
      
      m_topHalfStripLength_endcap->Fill(spacePoint->topHalfStripLength(), beamSpotWeight);
      m_bottomHalfStripLength_endcap->Fill(spacePoint->bottomHalfStripLength(), beamSpotWeight);

      m_topHalfStripLength_endcap->Fill(spacePoint->topHalfStripLength(), beamSpotWeight);
      m_bottomHalfStripLength_endcap->Fill(spacePoint->bottomHalfStripLength(), beamSpotWeight);

      for(int i(0); i<3; ++i) {
        m_topStripDirection_endcap->Fill(topStripDirection(i,0), beamSpotWeight);
        m_bottomStripDirection_endcap->Fill(bottomStripDirection(i,0), beamSpotWeight);
        m_stripCenterDistance_endcap->Fill(stripCenterDistance(i,0), beamSpotWeight);
        m_topStripCenter_endcap->Fill(topStripCenter(i,0), beamSpotWeight);
      } 
    } // barrel - endcap

  }
}
