/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "ALFA_DetectorTool.h"
#include "ALFA_DetectorFactory.h" 
#include "ALFA_GeoModel/ALFA_DetectorManager.h" 

#include "GeoModelUtilities/GeoModelExperiment.h"

#include "GaudiKernel/IService.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/MsgStream.h"

#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelUtilities/StoredPhysVol.h"

#include "StoreGate/StoreGateSvc.h"
#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"




using namespace std;

ALFA_DetectorTool::ALFA_DetectorTool( const std::string& type, 
				      const std::string& name, 
				      const IInterface* parent )
  : GeoModelTool( type, name, parent)
{
}

ALFA_DetectorTool::~ALFA_DetectorTool() = default;

void ALFA_DetectorTool::fillConfig()
{
	m_Config.clear();

	m_Config.GeometryConfig.eRPMetrologyGeoType=m_metrologyType;
	m_Config.GeometryConfig.strRPMetrologyConnString=m_metrologySource.value();
	m_Config.GeometryConfig.bShiftToX97Pos=m_shiftToX97Pos;
	m_Config.bConstructBeampipe=m_constructBeampipe;
	m_Config.bAddIBP=m_addBPInStation;

	m_Config.GeometryConfig.fNominalZPosA7L1=m_nominalZPosA7L1;
	m_Config.GeometryConfig.fNominalZPosB7L1=m_nominalZPosB7L1;
	m_Config.GeometryConfig.fNominalZPosA7R1=m_nominalZPosA7R1;
	m_Config.GeometryConfig.fNominalZPosB7R1=m_nominalZPosB7R1;

	for(int i=0;i<RPOTSCNT;i++){
		CFGRPPOSPARAMS& params=m_Config.GeometryConfig.CfgRPosParams[i];
		params.eRPPosType=m_rpPosType[i];
		params.eMDGeoType=m_mdGeoType[i];
		params.eODGeoType=m_odGeoType[i];
		params.strMDConnString=m_mdConnString[i].value();
		params.strODConnString=m_odConnString[i].value();
		params.fCurrentLVDTmm=m_currentLVDTmm[i];
	}

	m_Config.bIsTransformInStation=m_bIsTransformInStation.value();
	m_Config.bIsTransformInDetector=m_bIsTransformInDetector.value();

	m_Config.pointTransformInDetectorB7L1U=m_pointTransformInDetectorB7L1U.value();
	m_Config.pointTransformInDetectorB7L1L=m_pointTransformInDetectorB7L1L.value();
	m_Config.pointTransformInDetectorA7L1U=m_pointTransformInDetectorA7L1U.value();
	m_Config.pointTransformInDetectorA7L1L=m_pointTransformInDetectorA7L1L.value();
	m_Config.pointTransformInDetectorA7R1U=m_pointTransformInDetectorA7R1U.value();
	m_Config.pointTransformInDetectorA7R1L=m_pointTransformInDetectorA7R1L.value();
	m_Config.pointTransformInDetectorB7R1U=m_pointTransformInDetectorB7R1U.value();
	m_Config.pointTransformInDetectorB7R1L=m_pointTransformInDetectorB7R1L.value();

	m_Config.vecTransformInDetectorB7L1U=m_vecTransformInDetectorB7L1U.value();
	m_Config.vecTransformInDetectorB7L1L=m_vecTransformInDetectorB7L1L.value();
	m_Config.vecTransformInDetectorA7L1U=m_vecTransformInDetectorA7L1U.value();
	m_Config.vecTransformInDetectorA7L1L=m_vecTransformInDetectorA7L1L.value();
	m_Config.vecTransformInDetectorA7R1U=m_vecTransformInDetectorA7R1U.value();
	m_Config.vecTransformInDetectorA7R1L=m_vecTransformInDetectorA7R1L.value();
	m_Config.vecTransformInDetectorB7R1U=m_vecTransformInDetectorB7R1U.value();
	m_Config.vecTransformInDetectorB7R1L=m_vecTransformInDetectorB7R1L.value();

	m_Config.vecTransformInStationB7L1U=m_vecTransformInStationB7L1U.value();
	m_Config.vecTransformInStationB7L1L=m_vecTransformInStationB7L1L.value();
	m_Config.vecTransformInStationA7L1U=m_vecTransformInStationA7L1U.value();
	m_Config.vecTransformInStationA7L1L=m_vecTransformInStationA7L1L.value();
	m_Config.vecTransformInStationA7R1U=m_vecTransformInStationA7R1U.value();
	m_Config.vecTransformInStationA7R1L=m_vecTransformInStationA7R1L.value();
	m_Config.vecTransformInStationB7R1U=m_vecTransformInStationB7R1U.value();
	m_Config.vecTransformInStationB7R1L=m_vecTransformInStationB7R1L.value();
}

StatusCode ALFA_DetectorTool::create()
{ 
  ATH_MSG_INFO("Building ALFA_ geometry");

  fillConfig();

  if(((eMetrologyType)m_Config.GeometryConfig.eRPMetrologyGeoType)==EMT_SWCORRECTIONS){
	  CHECK(m_iovDbSvc.retrieve());
  }

  // Retrieve GeoModel Experiment
  GeoModelExperiment * theExpt{nullptr};
  ATH_CHECK(detStore()->retrieve(theExpt, "ATLAS"));
  
  GeoPhysVol *world=theExpt->getPhysVol();

  // Get pointer to the RDBAccessSvc
  // Use this pointer later for Geometry DB access
  SmartIF<IRDBAccessSvc> raccess{Gaudi::svcLocator()->service("RDBAccessSvc")};
  ATH_CHECK(raccess.isValid());

  // Construct Factory
  m_pALFADetectorFactory=new ALFA_DetectorFactory(detStore().operator->(),raccess,&m_Config);

  // Build geometry
  m_pALFADetectorFactory->create(world);

  // Add ALFA_ manager to the Store Gate and GeoModel Experiment
  theExpt->addManager(m_pALFADetectorFactory->getDetectorManager());
  ATH_CHECK(detStore()->record(m_pALFADetectorFactory->getDetectorManager()
			       , m_pALFADetectorFactory->getDetectorManager()->getName()));
    
  return StatusCode::SUCCESS;
}

StatusCode ALFA_DetectorTool::align()
{
	int nChannel;
	StatusCode sc=StatusCode::SUCCESS;
	const CondAttrListCollection* listAttrColl;
	CondAttrListCollection::const_iterator iterAttr;

	ALIGNPARAMETERS AlignParams;

	if(((eMetrologyType)m_Config.GeometryConfig.eRPMetrologyGeoType)==EMT_SWCORRECTIONS){
		// debug printout of global positions:
		StoredPhysVol* pStPhysRPBox=nullptr;
		sc=detStore()->retrieve(pStPhysRPBox,"StRPBox[03]");
		if(sc.isSuccess()){
			const GeoFullPhysVol* pPhysRPBox=pStPhysRPBox->getPhysVol();
			const GeoTrf::Transform3D& xf= pPhysRPBox->getAbsoluteTransform();

			ATH_MSG_INFO("Translation of RPBOX: "<< xf.translation());
		}

		if(detStore()->retrieve(listAttrColl,COOLFOLDER_DETSWCORR )==StatusCode::SUCCESS){
			for(iterAttr=listAttrColl->begin();iterAttr!=listAttrColl->end();++iterAttr){
				 nChannel=iterAttr->first; //RPot ID
				 AlignParams.fXOffset[nChannel]=((iterAttr->second)[0]).data<float>();
				 AlignParams.fTheta[nChannel]=((iterAttr->second)[1]).data<float>();
				 AlignParams.fYOffset[nChannel]=((iterAttr->second)[2]).data<float>();
			}

			m_pALFADetectorFactory->UpdateTransforms(&AlignParams);

			// debug printout of global positions:
			sc=detStore()->retrieve(pStPhysRPBox,"StRPBox[03]");
			if(sc.isSuccess()){
				const GeoFullPhysVol* pPhysRPBox=pStPhysRPBox->getPhysVol();
				const GeoTrf::Transform3D& xf= pPhysRPBox->getAbsoluteTransform();
				ATH_MSG_INFO("Translation of RPBOX after update: "<< xf.translation());
			}
		}
		else{
		        ATH_MSG_ERROR("Folder '"<<"/FWD/ALFA/position_calibration"<<"' not found");
			sc=StatusCode::FAILURE;
		}
	}

	return sc;
}
