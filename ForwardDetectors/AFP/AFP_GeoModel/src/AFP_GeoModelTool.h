/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef AFP_GeoModelTool_H
#define AFP_GeoModelTool_H

#include "GeoModelUtilities/GeoModelTool.h"
#include "AFP_GeoModelFactory.h"
#include "AFP_Geometry/AFP_Geometry.h"
#include "AFP_Geometry/AFP_constants.h"

#include <array>
#include <vector>


class AFP_GeoModelTool final : public GeoModelTool
{

 public:
    using GeoModelTool::GeoModelTool;
    virtual ~AFP_GeoModelTool() override final;
    
    virtual StatusCode create() override final;
    
 private:
    AFP_CONFIGURATION m_CfgParams; // filled from the properties in create()
    AFP_Geometry* m_pGeometry{nullptr};
    std::unique_ptr<AFP_GeoModelFactory> m_pAFPDetectorFactory;
    std::vector<double> m_vecAFP00XStaggering = std::vector<double>(AFP_CONSTANTS::SiT_Plate_amount, AFP_CONSTANTS::SiT_FarDistanceToFloor);
    std::vector<double> m_vecAFP00YStaggering = std::vector<double>(AFP_CONSTANTS::SiT_Plate_amount, 0.0*CLHEP::mm);
    std::vector<double> m_vecAFP01XStaggering = std::vector<double>(AFP_CONSTANTS::SiT_Plate_amount, AFP_CONSTANTS::SiT_NearDistanceToFloor);
    std::vector<double> m_vecAFP01YStaggering = std::vector<double>(AFP_CONSTANTS::SiT_Plate_amount, 0.0*CLHEP::mm);
    std::vector<double> m_vecAFP02XStaggering = std::vector<double>(AFP_CONSTANTS::SiT_Plate_amount, AFP_CONSTANTS::SiT_NearDistanceToFloor);
    std::vector<double> m_vecAFP02YStaggering = std::vector<double>(AFP_CONSTANTS::SiT_Plate_amount, 0.0*CLHEP::mm);
    std::vector<double> m_vecAFP03XStaggering = std::vector<double>(AFP_CONSTANTS::SiT_Plate_amount, AFP_CONSTANTS::SiT_FarDistanceToFloor);
    std::vector<double> m_vecAFP03YStaggering = std::vector<double>(AFP_CONSTANTS::SiT_Plate_amount, 0.0*CLHEP::mm);

    //Properties of SID
    BooleanProperty m_addVacuumSensors{this, "SID_AddVacuumSensors", false};

    //Properties of stations
    std::array<DoubleProperty, 4> m_rpotFloorDistance{{
      {this, "AFP00_RPotFloorDistance", AFP_CONSTANTS::Stat_RPotFloorDistance},
      {this, "AFP01_RPotFloorDistance", AFP_CONSTANTS::Stat_RPotFloorDistance},
      {this, "AFP02_RPotFloorDistance", AFP_CONSTANTS::Stat_RPotFloorDistance},
      {this, "AFP03_RPotFloorDistance", AFP_CONSTANTS::Stat_RPotFloorDistance}}};
    std::array<DoubleProperty, 4> m_rpotYPos{{
      {this, "AFP00_RPotYPos", AFP_CONSTANTS::Stat_ShiftInYAxis},
      {this, "AFP01_RPotYPos", AFP_CONSTANTS::Stat_ShiftInYAxis},
      {this, "AFP02_RPotYPos", AFP_CONSTANTS::Stat_ShiftInYAxis},
      {this, "AFP03_RPotYPos", AFP_CONSTANTS::Stat_ShiftInYAxis}}};
    std::array<DoubleProperty, 4> m_statNominalZPos{{
      {this, "AFP00_ZPos", AFP_CONSTANTS::Stat_OuterZDistance},
      {this, "AFP01_ZPos", AFP_CONSTANTS::Stat_InnerZDistance},
      {this, "AFP02_ZPos", -AFP_CONSTANTS::Stat_InnerZDistance},
      {this, "AFP03_ZPos", -AFP_CONSTANTS::Stat_OuterZDistance}}};
    
 private:
    StatusCode checkPropertiesSettings();
};

#endif // AFP_GeoModelTool_H
