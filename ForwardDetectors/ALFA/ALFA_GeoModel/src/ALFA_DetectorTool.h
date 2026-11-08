/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ALFA_DETECTORTOOL_H
#define ALFA_DETECTORTOOL_H

#include "GeoModelUtilities/GeoModelTool.h"
#include "ALFA_DetectorFactory.h" //typedef CONFIGURATION
#include "AthenaKernel/IIOVDbSvc.h"
#include "CLHEP/Vector/ThreeVector.h" //CLHEP::Hep3Vector
#include "CLHEP/Units/SystemOfUnits.h"
#include "Gaudi/Property.h"

#include <string>
#include <vector>



#define COOLFOLDER_DETSWCORR "/FWD/ALFA/position_calibration"

typedef struct _USERTRANSFORM
{
	int iRPot{};
	double fAngle{};
	CLHEP::Hep3Vector vecRotation;
	CLHEP::Hep3Vector vecTranslation;
} USERTRANSFORM, *PUSERTRANSFORM;


// Not thread-safe due to conditions callbacks.
class ATLAS_NOT_THREAD_SAFE ALFA_DetectorTool final : public GeoModelTool 
{
 private:
  CONFIGURATION m_Config{};
  ALFA_DetectorFactory* m_pALFADetectorFactory{nullptr};
  ServiceHandle< IIOVDbSvc > m_iovDbSvc{this, "IOVDbSvc", "IOVDbSvc"};

  // Properties copied into m_Config by fillConfig()
  Gaudi::Property<int> m_metrologyType{this, "MetrologyType", EMT_NOMINAL};//EMT_METROLOGY
  Gaudi::Property<std::string> m_metrologySource{this, "MetrologySource", ""};//"rpmetrology.dat"
  Gaudi::Property<bool> m_shiftToX97Pos{this, "ShiftToX97Pos", false};
  Gaudi::Property<bool> m_constructBeampipe{this, "ConstructBeampipe", false};
  Gaudi::Property<bool> m_addBPInStation{this, "AddBPInStation", true};

  Gaudi::Property<double> m_nominalZPosA7L1{this, "NominalZPosA7L1", 237388*CLHEP::mm};
  Gaudi::Property<double> m_nominalZPosB7L1{this, "NominalZPosB7L1", 241528*CLHEP::mm};
  Gaudi::Property<double> m_nominalZPosA7R1{this, "NominalZPosA7R1", -237408*CLHEP::mm};
  Gaudi::Property<double> m_nominalZPosB7R1{this, "NominalZPosB7R1", -241548*CLHEP::mm};

  // Per Roman Pot properties, indexed as CfgRPosParams
  Gaudi::Property<int> m_rpPosType[RPOTSCNT]{
    {this, "B7L1U_PosType", ERPPT_ACTIVE},
    {this, "B7L1L_PosType", ERPPT_ACTIVE},
    {this, "A7L1U_PosType", ERPPT_ACTIVE},
    {this, "A7L1L_PosType", ERPPT_ACTIVE},
    {this, "A7R1U_PosType", ERPPT_ACTIVE},
    {this, "A7R1L_PosType", ERPPT_ACTIVE},
    {this, "B7R1U_PosType", ERPPT_ACTIVE},
    {this, "B7R1L_PosType", ERPPT_ACTIVE}
  };
  Gaudi::Property<int> m_mdGeoType[RPOTSCNT]{
    {this, "B7L1U_MDGeometryType", EGST_IDEALGEOMETRY},
    {this, "B7L1L_MDGeometryType", EGST_IDEALGEOMETRY},
    {this, "A7L1U_MDGeometryType", EGST_IDEALGEOMETRY},
    {this, "A7L1L_MDGeometryType", EGST_IDEALGEOMETRY},
    {this, "A7R1U_MDGeometryType", EGST_IDEALGEOMETRY},
    {this, "A7R1L_MDGeometryType", EGST_IDEALGEOMETRY},
    {this, "B7R1U_MDGeometryType", EGST_IDEALGEOMETRY},
    {this, "B7R1L_MDGeometryType", EGST_IDEALGEOMETRY}
  };
  Gaudi::Property<int> m_odGeoType[RPOTSCNT]{
    {this, "B7L1U_ODGeometryType", EGST_IDEALGEOMETRY},
    {this, "B7L1L_ODGeometryType", EGST_IDEALGEOMETRY},
    {this, "A7L1U_ODGeometryType", EGST_IDEALGEOMETRY},
    {this, "A7L1L_ODGeometryType", EGST_IDEALGEOMETRY},
    {this, "A7R1U_ODGeometryType", EGST_IDEALGEOMETRY},
    {this, "A7R1L_ODGeometryType", EGST_IDEALGEOMETRY},
    {this, "B7R1U_ODGeometryType", EGST_IDEALGEOMETRY},
    {this, "B7R1L_ODGeometryType", EGST_IDEALGEOMETRY}
  };
  Gaudi::Property<std::string> m_mdConnString[RPOTSCNT]{
    {this, "B7L1U_MDSource", ""},
    {this, "B7L1L_MDSource", ""},
    {this, "A7L1U_MDSource", ""},
    {this, "A7L1L_MDSource", ""},
    {this, "A7R1U_MDSource", ""},
    {this, "A7R1L_MDSource", ""},
    {this, "B7R1U_MDSource", ""},
    {this, "B7R1L_MDSource", ""}
  };
  Gaudi::Property<std::string> m_odConnString[RPOTSCNT]{
    {this, "B7L1U_ODSource", ""},
    {this, "B7L1L_ODSource", ""},
    {this, "A7L1U_ODSource", ""},
    {this, "A7L1L_ODSource", ""},
    {this, "A7R1U_ODSource", ""},
    {this, "A7R1L_ODSource", ""},
    {this, "B7R1U_ODSource", ""},
    {this, "B7R1L_ODSource", ""}
  };
  Gaudi::Property<double> m_currentLVDTmm[RPOTSCNT]{
    {this, "B7L1U_CurrentLVDT", 7.0*CLHEP::mm},
    {this, "B7L1L_CurrentLVDT", -7.0*CLHEP::mm},
    {this, "A7L1U_CurrentLVDT", 7.0*CLHEP::mm},
    {this, "A7L1L_CurrentLVDT", -7.0*CLHEP::mm},
    {this, "A7R1U_CurrentLVDT", 7.0*CLHEP::mm},
    {this, "A7R1L_CurrentLVDT", -7.0*CLHEP::mm},
    {this, "B7R1U_CurrentLVDT", 7.0*CLHEP::mm},
    {this, "B7R1L_CurrentLVDT", -7.0*CLHEP::mm}
  };

  Gaudi::Property<std::vector<bool>> m_bIsTransformInStation{this, "bIsTransformInStation", std::vector<bool>(8, false), "status if transformation in station will be calculated"};
  Gaudi::Property<std::vector<bool>> m_bIsTransformInDetector{this, "bIsTransformInDetector", std::vector<bool>(8, false), "status if transformation in station will be calculated"};

  Gaudi::Property<std::vector<double>> m_pointTransformInDetectorB7L1U{this, "pointTransformInDetectorB7L1U", std::vector<double>(3, 0.0), "transformation point in the detectorfor B7L1U RP"};
  Gaudi::Property<std::vector<double>> m_pointTransformInDetectorB7L1L{this, "pointTransformInDetectorB7L1L", std::vector<double>(3, 0.0), "transformation point in the detectorfor B7L1L RP"};
  Gaudi::Property<std::vector<double>> m_pointTransformInDetectorA7L1U{this, "pointTransformInDetectorA7L1U", std::vector<double>(3, 0.0), "transformation point in the detectorfor A7L1U RP"};
  Gaudi::Property<std::vector<double>> m_pointTransformInDetectorA7L1L{this, "pointTransformInDetectorA7L1L", std::vector<double>(3, 0.0), "transformation point in the detectorfor A7L1L RP"};
  Gaudi::Property<std::vector<double>> m_pointTransformInDetectorA7R1U{this, "pointTransformInDetectorA7R1U", std::vector<double>(3, 0.0), "transformation point in the detectorfor A7R1U RP"};
  Gaudi::Property<std::vector<double>> m_pointTransformInDetectorA7R1L{this, "pointTransformInDetectorA7R1L", std::vector<double>(3, 0.0), "transformation point in the detectorfor A7R1L RP"};
  Gaudi::Property<std::vector<double>> m_pointTransformInDetectorB7R1U{this, "pointTransformInDetectorB7R1U", std::vector<double>(3, 0.0), "transformation point in the detectorfor B7R1U RP"};
  Gaudi::Property<std::vector<double>> m_pointTransformInDetectorB7R1L{this, "pointTransformInDetectorB7R1L", std::vector<double>(3, 0.0), "transformation point in the detectorfor B7R1L RP"};

  Gaudi::Property<std::vector<double>> m_vecTransformInDetectorB7L1U{this, "vecTransformInDetectorB7L1U", std::vector<double>(7, 0.0), "transformation data in the detectorfor B7L1U RP"};
  Gaudi::Property<std::vector<double>> m_vecTransformInDetectorB7L1L{this, "vecTransformInDetectorB7L1L", std::vector<double>(7, 0.0), "transformation data in the detectorfor B7L1L RP"};
  Gaudi::Property<std::vector<double>> m_vecTransformInDetectorA7L1U{this, "vecTransformInDetectorA7L1U", std::vector<double>(7, 0.0), "transformation data in the detectorfor A7L1U RP"};
  Gaudi::Property<std::vector<double>> m_vecTransformInDetectorA7L1L{this, "vecTransformInDetectorA7L1L", std::vector<double>(7, 0.0), "transformation data in the detectorfor A7L1L RP"};
  Gaudi::Property<std::vector<double>> m_vecTransformInDetectorA7R1U{this, "vecTransformInDetectorA7R1U", std::vector<double>(7, 0.0), "transformation data in the detectorfor A7R1U RP"};
  Gaudi::Property<std::vector<double>> m_vecTransformInDetectorA7R1L{this, "vecTransformInDetectorA7R1L", std::vector<double>(7, 0.0), "transformation data in the detectorfor A7R1L RP"};
  Gaudi::Property<std::vector<double>> m_vecTransformInDetectorB7R1U{this, "vecTransformInDetectorB7R1U", std::vector<double>(7, 0.0), "transformation data in the detectorfor B7R1U RP"};
  Gaudi::Property<std::vector<double>> m_vecTransformInDetectorB7R1L{this, "vecTransformInDetectorB7R1L", std::vector<double>(7, 0.0), "transformation data in the detectorfor B7R1L RP"};

  Gaudi::Property<std::vector<double>> m_vecTransformInStationB7L1U{this, "vecTransformInStationB7L1U", std::vector<double>(7, 0.0), "transformation data in the stationfor B7L1U RP"};
  Gaudi::Property<std::vector<double>> m_vecTransformInStationB7L1L{this, "vecTransformInStationB7L1L", std::vector<double>(7, 0.0), "transformation data in the stationfor B7L1L RP"};
  Gaudi::Property<std::vector<double>> m_vecTransformInStationA7L1U{this, "vecTransformInStationA7L1U", std::vector<double>(7, 0.0), "transformation data in the stationfor A7L1U RP"};
  Gaudi::Property<std::vector<double>> m_vecTransformInStationA7L1L{this, "vecTransformInStationA7L1L", std::vector<double>(7, 0.0), "transformation data in the stationfor A7L1L RP"};
  Gaudi::Property<std::vector<double>> m_vecTransformInStationA7R1U{this, "vecTransformInStationA7R1U", std::vector<double>(7, 0.0), "transformation data in the stationfor A7R1U RP"};
  Gaudi::Property<std::vector<double>> m_vecTransformInStationA7R1L{this, "vecTransformInStationA7R1L", std::vector<double>(7, 0.0), "transformation data in the stationfor A7R1L RP"};
  Gaudi::Property<std::vector<double>> m_vecTransformInStationB7R1U{this, "vecTransformInStationB7R1U", std::vector<double>(7, 0.0), "transformation data in the stationfor B7R1U RP"};
  Gaudi::Property<std::vector<double>> m_vecTransformInStationB7R1L{this, "vecTransformInStationB7R1L", std::vector<double>(7, 0.0), "transformation data in the stationfor B7R1L RP"};

  void fillConfig();

 public:
  // Standard Constructor
  ALFA_DetectorTool( const std::string& type, const std::string& name, const IInterface* parent );

  // Standard Destructor
  virtual ~ALFA_DetectorTool() override final;

  // Build geometry and store Manager to the TDS
  virtual StatusCode create() override final;

  virtual StatusCode align() override final;
};

#endif
