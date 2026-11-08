/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ALFA_LOCRECCORR_h
#define ALFA_LOCRECCORR_h



#include "TROOT.h"

#include "AthenaBaseComps/AthAlgorithm.h"

#include "GaudiKernel/ServiceHandle.h"
#include "Gaudi/Property.h"
#include "CLHEP/Units/SystemOfUnits.h"
#include "StoreGate/StoreGateSvc.h"
#include "GeneratorObjects/McEventCollection.h"
#include "AthenaPoolUtilities/AthenaAttributeList.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"

#include "ALFA_RawEv/ALFA_RawData.h"
#include "ALFA_RawEv/ALFA_RawDataContainer.h"
#include "ALFA_RawEv/ALFA_RawDataCollection.h"
#include "ALFA_RawEv/ALFA_DigitCollection.h"
#include "ALFA_RawEv/ALFA_ODDigitCollection.h"
#include "ALFA_Geometry/ALFA_GeometryReader.h"
#include "ALFA_Geometry/ALFA_constants.h"
#include "ALFA_LocRecEv/ALFA_LocRecEvCollection.h"
#include "ALFA_LocRecEv/ALFA_LocRecODEvCollection.h"
#include "ALFA_LocRecCorrEv/ALFA_LocRecCorrEvCollection.h"
#include "ALFA_LocRecCorrEv/ALFA_LocRecCorrODEvCollection.h"
#include "ALFA_UserObjects.h"
#include <string>
#include <list>
#include <vector>

#define MAXNUMTRACKS 100

typedef struct _USERTRANSFORM
{
	int iRPot{};
	double fAngle{};
	CLHEP::Hep3Vector vecRotation;
	CLHEP::Hep3Vector vecTranslation;

} USERTRANSFORM, *PUSERTRANSFORM;


class StoreGateSvc;

class ALFA_LocRecCorr : public AthAlgorithm
{
public:
	ALFA_LocRecCorr(const std::string& name, ISvcLocator* pSvcLocator);
	~ALFA_LocRecCorr();

private:
	GEOMETRYCONFIGURATION m_Config;
	ALFA_GeometryReader* m_pGeometryReader{nullptr};

	// a handle on Store Gate
	//StoreGateSvc* m_storeGate;
	//StoreGateSvc* m_pDetStore;

	ALFA_LocRecCorrEvCollection*	m_pLocRecCorrEvCollection{nullptr};
	ALFA_LocRecCorrODEvCollection*	m_pLocRecCorrODEvCollection{nullptr};

	//begin of Geometry properties (copied into m_Config in initialize())
	Gaudi::Property<int> m_MetrologyType{this, "MetrologyType", EMT_NOMINAL};//EGST_FILE
	Gaudi::Property<std::string> m_MetrologySource{this, "MetrologySource", ""};//"rpmetrology.dat"
	Gaudi::Property<bool> m_ShiftToX97Pos{this, "ShiftToX97Pos", false};

	Gaudi::Property<int> m_PosType[RPOTSCNT]{
		{this, "B7L1U_PosType", ERPPT_ACTIVE},
		{this, "B7L1L_PosType", ERPPT_ACTIVE},
		{this, "A7L1U_PosType", ERPPT_ACTIVE},
		{this, "A7L1L_PosType", ERPPT_ACTIVE},
		{this, "A7R1U_PosType", ERPPT_ACTIVE},
		{this, "A7R1L_PosType", ERPPT_ACTIVE},
		{this, "B7R1U_PosType", ERPPT_ACTIVE},
		{this, "B7R1L_PosType", ERPPT_ACTIVE}
	};
	Gaudi::Property<int> m_MDGeometryType[RPOTSCNT]{
		{this, "B7L1U_MDGeometryType", EGST_IDEALGEOMETRY},
		{this, "B7L1L_MDGeometryType", EGST_IDEALGEOMETRY},
		{this, "A7L1U_MDGeometryType", EGST_IDEALGEOMETRY},
		{this, "A7L1L_MDGeometryType", EGST_IDEALGEOMETRY},
		{this, "A7R1U_MDGeometryType", EGST_IDEALGEOMETRY},
		{this, "A7R1L_MDGeometryType", EGST_IDEALGEOMETRY},
		{this, "B7R1U_MDGeometryType", EGST_IDEALGEOMETRY},
		{this, "B7R1L_MDGeometryType", EGST_IDEALGEOMETRY}
	};
	Gaudi::Property<int> m_ODGeometryType[RPOTSCNT]{
		{this, "B7L1U_ODGeometryType", EGST_IDEALGEOMETRY},
		{this, "B7L1L_ODGeometryType", EGST_IDEALGEOMETRY},
		{this, "A7L1U_ODGeometryType", EGST_IDEALGEOMETRY},
		{this, "A7L1L_ODGeometryType", EGST_IDEALGEOMETRY},
		{this, "A7R1U_ODGeometryType", EGST_IDEALGEOMETRY},
		{this, "A7R1L_ODGeometryType", EGST_IDEALGEOMETRY},
		{this, "B7R1U_ODGeometryType", EGST_IDEALGEOMETRY},
		{this, "B7R1L_ODGeometryType", EGST_IDEALGEOMETRY}
	};
	Gaudi::Property<std::string> m_MDSource[RPOTSCNT]{
		{this, "B7L1U_MDSource", ""},
		{this, "B7L1L_MDSource", ""},
		{this, "A7L1U_MDSource", ""},
		{this, "A7L1L_MDSource", ""},
		{this, "A7R1U_MDSource", ""},
		{this, "A7R1L_MDSource", ""},
		{this, "B7R1U_MDSource", ""},
		{this, "B7R1L_MDSource", ""}
	};
	Gaudi::Property<std::string> m_ODSource[RPOTSCNT]{
		{this, "B7L1U_ODSource", ""},
		{this, "B7L1L_ODSource", ""},
		{this, "A7L1U_ODSource", ""},
		{this, "A7L1L_ODSource", ""},
		{this, "A7R1U_ODSource", ""},
		{this, "A7R1L_ODSource", ""},
		{this, "B7R1U_ODSource", ""},
		{this, "B7R1L_ODSource", ""}
	};
	Gaudi::Property<double> m_CurrentLVDT[RPOTSCNT]{
		{this, "B7L1U_CurrentLVDT", 7.0*CLHEP::mm},
		{this, "B7L1L_CurrentLVDT", 7.0*CLHEP::mm},
		{this, "A7L1U_CurrentLVDT", 7.0*CLHEP::mm},
		{this, "A7L1L_CurrentLVDT", 7.0*CLHEP::mm},
		{this, "A7R1U_CurrentLVDT", 7.0*CLHEP::mm},
		{this, "A7R1L_CurrentLVDT", 7.0*CLHEP::mm},
		{this, "B7R1U_CurrentLVDT", 7.0*CLHEP::mm},
		{this, "B7R1L_CurrentLVDT", 7.0*CLHEP::mm}
	};
	//end of Geometry properties

	Gaudi::Property<bool> m_bCoolData{this, "CoolData", true};
	std::list<eRPotName> m_ListExistingRPots;

	std::string m_strLocRecCollectionName{"ALFA_LocRecEvCollection"};
	std::string m_strLocRecODCollectionName{"ALFA_LocRecODEvCollection"};

	// data type using in the local reconstruction
	// for the simulation data the value is 0, for the real data the value is 1. Unset value is -1
	Gaudi::Property<Int_t> m_iDataType{this, "DataType", 1, "data type using in the local reconstruction"};	//data type (simulation or real data) using in the local reconstruction
	Int_t m_iEvt{0};

	//for transformations
	Gaudi::Property<std::vector<bool>> m_bIsTransformInStation{this, "bIsTransformInStation", std::vector<bool>(8, false), "status if transformation in station will be calculated"};
	Gaudi::Property<std::vector<bool>> m_bIsTransformInDetector{this, "bIsTransformInDetector", std::vector<bool>(8, false), "status if transformation in station will be calculated"};
	Gaudi::Property<std::vector<double>> m_pointTransformInDetectorB7L1U{this, "pointTransformInDetectorB7L1U", std::vector<double>(3, 0.0), "transformation point in the detectorfor B7L1U RP"};
	Gaudi::Property<std::vector<double>> m_pointTransformInDetectorB7L1L{this, "pointTransformInDetectorB7L1L", std::vector<double>(3, 0.0), "transformation point in the detectorfor B7L1L RP"};
	Gaudi::Property<std::vector<double>> m_pointTransformInDetectorA7L1U{this, "pointTransformInDetectorA7L1U", std::vector<double>(3, 0.0), "transformation point in the detectorfor A7L1U RP"};
	Gaudi::Property<std::vector<double>> m_pointTransformInDetectorA7L1L{this, "pointTransformInDetectorA7L1L", std::vector<double>(3, 0.0), "transformation point in the detectorfor A7L1L RP"};
	Gaudi::Property<std::vector<double>> m_pointTransformInDetectorB7R1U{this, "pointTransformInDetectorB7R1U", std::vector<double>(3, 0.0), "transformation point in the detectorfor B7R1U RP"};
	Gaudi::Property<std::vector<double>> m_pointTransformInDetectorB7R1L{this, "pointTransformInDetectorB7R1L", std::vector<double>(3, 0.0), "transformation point in the detectorfor B7R1L RP"};
	Gaudi::Property<std::vector<double>> m_pointTransformInDetectorA7R1U{this, "pointTransformInDetectorA7R1U", std::vector<double>(3, 0.0), "transformation point in the detectorfor A7R1U RP"};
	Gaudi::Property<std::vector<double>> m_pointTransformInDetectorA7R1L{this, "pointTransformInDetectorA7R1L", std::vector<double>(3, 0.0), "transformation point in the detectorfor A7R1L RP"};
	Gaudi::Property<std::vector<double>> m_vecTransformInDetectorB7L1U{this, "vecTransformInDetectorB7L1U", std::vector<double>(7, 0.0), "transformation data in the detectorfor B7L1U RP"};
	Gaudi::Property<std::vector<double>> m_vecTransformInDetectorB7L1L{this, "vecTransformInDetectorB7L1L", std::vector<double>(7, 0.0), "transformation data in the detectorfor B7L1L RP"};
	Gaudi::Property<std::vector<double>> m_vecTransformInDetectorA7L1U{this, "vecTransformInDetectorA7L1U", std::vector<double>(7, 0.0), "transformation data in the detectorfor A7L1U RP"};
	Gaudi::Property<std::vector<double>> m_vecTransformInDetectorA7L1L{this, "vecTransformInDetectorA7L1L", std::vector<double>(7, 0.0), "transformation data in the detectorfor A7L1L RP"};
	Gaudi::Property<std::vector<double>> m_vecTransformInDetectorB7R1U{this, "vecTransformInDetectorB7R1U", std::vector<double>(7, 0.0), "transformation data in the detectorfor B7R1U RP"};
	Gaudi::Property<std::vector<double>> m_vecTransformInDetectorB7R1L{this, "vecTransformInDetectorB7R1L", std::vector<double>(7, 0.0), "transformation data in the detectorfor B7R1L RP"};
	Gaudi::Property<std::vector<double>> m_vecTransformInDetectorA7R1U{this, "vecTransformInDetectorA7R1U", std::vector<double>(7, 0.0), "transformation data in the detectorfor A7R1U RP"};
	Gaudi::Property<std::vector<double>> m_vecTransformInDetectorA7R1L{this, "vecTransformInDetectorA7R1L", std::vector<double>(7, 0.0), "transformation data in the detectorfor A7R1L RP"};
	Gaudi::Property<std::vector<double>> m_vecTransformInStationB7L1U{this, "vecTransformInStationB7L1U", std::vector<double>(7, 0.0), "transformation data in the stationfor B7L1U RP"};
	Gaudi::Property<std::vector<double>> m_vecTransformInStationB7L1L{this, "vecTransformInStationB7L1L", std::vector<double>(7, 0.0), "transformation data in the stationfor B7L1L RP"};
	Gaudi::Property<std::vector<double>> m_vecTransformInStationA7L1U{this, "vecTransformInStationA7L1U", std::vector<double>(7, 0.0), "transformation data in the stationfor A7L1U RP"};
	Gaudi::Property<std::vector<double>> m_vecTransformInStationA7L1L{this, "vecTransformInStationA7L1L", std::vector<double>(7, 0.0), "transformation data in the stationfor A7L1L RP"};
	Gaudi::Property<std::vector<double>> m_vecTransformInStationB7R1U{this, "vecTransformInStationB7R1U", std::vector<double>(7, 0.0), "transformation data in the stationfor B7R1U RP"};
	Gaudi::Property<std::vector<double>> m_vecTransformInStationB7R1L{this, "vecTransformInStationB7R1L", std::vector<double>(7, 0.0), "transformation data in the stationfor B7R1L RP"};
	Gaudi::Property<std::vector<double>> m_vecTransformInStationA7R1U{this, "vecTransformInStationA7R1U", std::vector<double>(7, 0.0), "transformation data in the stationfor A7R1U RP"};
	Gaudi::Property<std::vector<double>> m_vecTransformInStationA7R1L{this, "vecTransformInStationA7R1L", std::vector<double>(7, 0.0), "transformation data in the stationfor A7R1L RP"};

	std::string m_strKeyGeometryForReco{"ALFA_GeometryForReco"};
	std::string m_strKeyLocRecEvCollection{"ALFA_LocRecEvCollection"};
	std::string m_strKeyLocRecODEvCollection{"ALFA_LocRecODEvCollection"};
	std::string m_strKeyLocRecCorrEvCollection{"ALFA_LocRecCorrEvCollection"};
	std::string m_strKeyLocRecCorrODEvCollection{"ALFA_LocRecCorrODEvCollection"};
	std::string m_strCollectionName;
	std::string m_strODCollectionName;
	std::string m_strTruthCollectionName;
	std::string m_strKeyRawDataCollection;
	std::string m_rootInputFileName;

	HepGeom::Transform3D m_TransMatrixSt[RPOTSCNT];
	HepGeom::Transform3D m_TransMatrixLHC[RPOTSCNT];

public:
	StatusCode initialize();
	StatusCode execute(const EventContext& ctx);

private:
	bool UpdateGeometryAtlas();
	void SetNominalGeometry();

	StatusCode RecordCollection();
	StatusCode RecordODCollection();

	HepGeom::Transform3D UserTransform3DInDetector(eRPotName eRPName);
	HepGeom::Transform3D UserTransform3DInStation(eRPotName eRPName);
	HepGeom::Point3D<double> Point3DInDetector(eRPotName eRPName);

	StatusCode COOLUpdate();
};

#endif	//ALFA_LOCRECCORR_h
