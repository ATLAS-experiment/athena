/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ALFA_LOCRECCORR_h
#define ALFA_LOCRECCORR_h

#include <iostream>
#include <string>
#include <list>
#include <map>
#include <vector>

#include "Riostream.h"
#include "TROOT.h"

#include "AthenaBaseComps/AthAlgorithm.h"

#include "GaudiKernel/ServiceHandle.h"
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

#define MAXNUMTRACKS 100

typedef struct _USERTRANSFORM
{
	int iRPot;
	double fAngle;
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
	ALFA_GeometryReader* m_pGeometryReader;

	// a handle on Store Gate
	//StoreGateSvc* m_storeGate;
	//StoreGateSvc* m_pDetStore;

	ALFA_LocRecCorrEvCollection*	m_pLocRecCorrEvCollection;
	ALFA_LocRecCorrODEvCollection*	m_pLocRecCorrODEvCollection;

	bool m_bCoolData;
	std::list<eRPotName> m_ListExistingRPots;

	std::string m_strLocRecCollectionName;
	std::string m_strLocRecODCollectionName;

	Int_t m_iDataType;			//data type (simulation or real data) using in the local reconstruction
	Int_t m_iEvt;

	std::vector<bool> m_bIsTransformInStation;
	std::vector<bool> m_bIsTransformInDetector;
	std::vector<double> m_pointTransformInDetectorB7L1U;
	std::vector<double> m_pointTransformInDetectorB7L1L;
	std::vector<double> m_pointTransformInDetectorA7L1U;
	std::vector<double> m_pointTransformInDetectorA7L1L;
	std::vector<double> m_pointTransformInDetectorB7R1U;
	std::vector<double> m_pointTransformInDetectorB7R1L;
	std::vector<double> m_pointTransformInDetectorA7R1U;
	std::vector<double> m_pointTransformInDetectorA7R1L;
	std::vector<double> m_vecTransformInDetectorB7L1U;
	std::vector<double> m_vecTransformInDetectorB7L1L;
	std::vector<double> m_vecTransformInDetectorA7L1U;
	std::vector<double> m_vecTransformInDetectorA7L1L;
	std::vector<double> m_vecTransformInDetectorB7R1U;
	std::vector<double> m_vecTransformInDetectorB7R1L;
	std::vector<double> m_vecTransformInDetectorA7R1U;
	std::vector<double> m_vecTransformInDetectorA7R1L;
	std::vector<double> m_vecTransformInStationB7L1U;
	std::vector<double> m_vecTransformInStationB7L1L;
	std::vector<double> m_vecTransformInStationA7L1U;
	std::vector<double> m_vecTransformInStationA7L1L;
	std::vector<double> m_vecTransformInStationB7R1U;
	std::vector<double> m_vecTransformInStationB7R1L;
	std::vector<double> m_vecTransformInStationA7R1U;
	std::vector<double> m_vecTransformInStationA7R1L;

	std::string m_strKeyGeometryForReco;
	std::string m_strKeyLocRecEvCollection;
	std::string m_strKeyLocRecODEvCollection;
	std::string m_strKeyLocRecCorrEvCollection;
	std::string m_strKeyLocRecCorrODEvCollection;
	std::string m_strCollectionName;
	std::string m_strODCollectionName;
	std::string m_strTruthCollectionName;
	std::string m_strKeyRawDataCollection;
	std::string m_rootInputFileName;

	HepGeom::Transform3D m_TransMatrixSt[RPOTSCNT];
	HepGeom::Transform3D m_TransMatrixLHC[RPOTSCNT];

public:
	StatusCode initialize();
	StatusCode execute();

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
