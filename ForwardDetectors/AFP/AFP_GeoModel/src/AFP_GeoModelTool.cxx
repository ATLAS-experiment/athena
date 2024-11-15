/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "GeoModelUtilities/GeoModelExperiment.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "AthenaPoolUtilities/AthenaAttributeList.h"

#include "AFP_GeoModelTool.h"
#include "AFP_GeoModelFactory.h"
#include "AFP_GeoModelManager.h"
#include "AFP_Geometry/AFP_constants.h"

/**
 ** Constructor(s)
 **/
AFP_GeoModelTool::AFP_GeoModelTool( const std::string& type, const std::string& name, const IInterface* parent )
    : GeoModelTool( type, name, parent )
{
    m_CfgParams.clear();
    m_pGeometry=nullptr;

	m_defsidcfg.clear();

	m_vecAFP00XStaggering.assign(AFP_CONSTANTS::SiT_Plate_amount,AFP_CONSTANTS::SiT_FarDistanceToFloor);
	m_vecAFP00YStaggering.assign(AFP_CONSTANTS::SiT_Plate_amount,0.0*CLHEP::mm);
	m_vecAFP01XStaggering.assign(AFP_CONSTANTS::SiT_Plate_amount,AFP_CONSTANTS::SiT_NearDistanceToFloor);
	m_vecAFP01YStaggering.assign(AFP_CONSTANTS::SiT_Plate_amount,0.0*CLHEP::mm);
	m_vecAFP02XStaggering.assign(AFP_CONSTANTS::SiT_Plate_amount,AFP_CONSTANTS::SiT_NearDistanceToFloor);
	m_vecAFP02YStaggering.assign(AFP_CONSTANTS::SiT_Plate_amount,0.0*CLHEP::mm);
	m_vecAFP03XStaggering.assign(AFP_CONSTANTS::SiT_Plate_amount,AFP_CONSTANTS::SiT_FarDistanceToFloor);
	m_vecAFP03YStaggering.assign(AFP_CONSTANTS::SiT_Plate_amount,0.0*CLHEP::mm);

    //Properties of SID
	declareProperty("SID_AddVacuumSensors",m_defsidcfg.bAddVacuumSensors=false);


    //Properties of TD


    //Properties of stations
	declareProperty("AFP00_RPotFloorDistance",m_CfgParams.vecRPotFloorDistance[0]=AFP_CONSTANTS::Stat_RPotFloorDistance);
	declareProperty("AFP01_RPotFloorDistance",m_CfgParams.vecRPotFloorDistance[1]=AFP_CONSTANTS::Stat_RPotFloorDistance);
	declareProperty("AFP02_RPotFloorDistance",m_CfgParams.vecRPotFloorDistance[2]=AFP_CONSTANTS::Stat_RPotFloorDistance);
	declareProperty("AFP03_RPotFloorDistance",m_CfgParams.vecRPotFloorDistance[3]=AFP_CONSTANTS::Stat_RPotFloorDistance);

	declareProperty("AFP00_RPotYPos",m_CfgParams.vecRPotYPos[0]=AFP_CONSTANTS::Stat_ShiftInYAxis);
	declareProperty("AFP01_RPotYPos",m_CfgParams.vecRPotYPos[1]=AFP_CONSTANTS::Stat_ShiftInYAxis);
	declareProperty("AFP02_RPotYPos",m_CfgParams.vecRPotYPos[2]=AFP_CONSTANTS::Stat_ShiftInYAxis);
	declareProperty("AFP03_RPotYPos",m_CfgParams.vecRPotYPos[3]=AFP_CONSTANTS::Stat_ShiftInYAxis);

	declareProperty("AFP00_ZPos",m_CfgParams.vecStatNominalZPos[0]=AFP_CONSTANTS::Stat_OuterZDistance);
	declareProperty("AFP01_ZPos",m_CfgParams.vecStatNominalZPos[1]=AFP_CONSTANTS::Stat_InnerZDistance);
	declareProperty("AFP02_ZPos",m_CfgParams.vecStatNominalZPos[2]=-AFP_CONSTANTS::Stat_InnerZDistance);
	declareProperty("AFP03_ZPos",m_CfgParams.vecStatNominalZPos[3]=-AFP_CONSTANTS::Stat_OuterZDistance);

	m_CfgParams.sidcfg[EAS_AFP00]=m_defsidcfg;
	m_CfgParams.sidcfg[EAS_AFP01]=m_defsidcfg;
	m_CfgParams.sidcfg[EAS_AFP02]=m_defsidcfg;
	m_CfgParams.sidcfg[EAS_AFP03]=m_defsidcfg;
}

/**
 ** Destructor
 **/
AFP_GeoModelTool::~AFP_GeoModelTool()
{
    // This will need to be modified once we register the  DetectorNode in
    // the Transient Detector Store
    if(m_detector!=nullptr) {
        delete m_detector;
        m_detector=nullptr;
    }

    if(m_pGeometry!=nullptr){
        delete m_pGeometry;
        m_pGeometry=nullptr;
    }
}

StatusCode AFP_GeoModelTool::checkPropertiesSettings()
{
	bool bRes=true;

	if(!m_vecAFP00XStaggering.empty()){
		if(m_vecAFP00XStaggering.size()==m_CfgParams.sidcfg[EAS_AFP00].fLayerCount){
			m_CfgParams.sidcfg[EAS_AFP00].vecXStaggering=m_vecAFP00XStaggering;
		}
		else{
			ATH_MSG_ERROR("Mismatch between SID_AFP01XStaggering and number of plates (SID_NumberOfLayers)");
			bRes=false;
		}
	}
	if(!m_vecAFP00YStaggering.empty()){
		if(m_vecAFP00YStaggering.size()==m_CfgParams.sidcfg[EAS_AFP00].fLayerCount){
			m_CfgParams.sidcfg[EAS_AFP00].vecYStaggering=m_vecAFP00YStaggering;
		}
		else{
			ATH_MSG_ERROR("Mismatch between SID_AFP00YStaggering and number of plates (SID_NumberOfLayers)");
			bRes=false;
		}
	}

	if(!m_vecAFP01XStaggering.empty()){
		if(m_vecAFP01XStaggering.size()==m_CfgParams.sidcfg[EAS_AFP01].fLayerCount){
			m_CfgParams.sidcfg[EAS_AFP01].vecXStaggering=m_vecAFP01XStaggering;
		}
		else{
			ATH_MSG_ERROR("Mismatch between SID_AFP01XStaggering and number of plates (SID_NumberOfLayers)");
			bRes=false;
		}
	}
	if(!m_vecAFP01YStaggering.empty()){
		if(m_vecAFP01YStaggering.size()==m_CfgParams.sidcfg[EAS_AFP01].fLayerCount){
			m_CfgParams.sidcfg[EAS_AFP01].vecYStaggering=m_vecAFP01YStaggering;
		}
		else{
			ATH_MSG_ERROR("Mismatch between SID_AFP01YStaggering and number of plates (SID_NumberOfLayers)");
			bRes=false;
		}
	}

	if(!m_vecAFP02XStaggering.empty()){
		if(m_vecAFP02XStaggering.size()==m_CfgParams.sidcfg[EAS_AFP02].fLayerCount){
			m_CfgParams.sidcfg[EAS_AFP02].vecXStaggering=m_vecAFP02XStaggering;
		}
		else{
			ATH_MSG_ERROR("Mismatch between SID_AFP02XStaggering and number of plates (SID_NumberOfLayers)");
			bRes=false;
		}
	}
	if(!m_vecAFP02YStaggering.empty()){
		if(m_vecAFP02YStaggering.size()==m_CfgParams.sidcfg[EAS_AFP02].fLayerCount){
			m_CfgParams.sidcfg[EAS_AFP02].vecYStaggering=m_vecAFP02YStaggering;
		}
		else{
			ATH_MSG_ERROR("Mismatch between SID_AFP02YStaggering and number of plates (SID_NumberOfLayers)");
			bRes=false;
		}
	}

	if(!m_vecAFP03XStaggering.empty()){
		if(m_vecAFP03XStaggering.size()==m_CfgParams.sidcfg[EAS_AFP03].fLayerCount){
			m_CfgParams.sidcfg[EAS_AFP03].vecXStaggering=m_vecAFP03XStaggering;
		}
		else{
			ATH_MSG_ERROR("Mismatch between SID_AFP03XStaggering and number of plates (SID_NumberOfLayers)");
			bRes=false;
		}
	}
	if(!m_vecAFP03YStaggering.empty()){
		if(m_vecAFP03YStaggering.size()==m_CfgParams.sidcfg[EAS_AFP03].fLayerCount){
			m_CfgParams.sidcfg[EAS_AFP03].vecYStaggering=m_vecAFP03YStaggering;
		}
		else{
			ATH_MSG_ERROR("Mismatch between SID_AFP03YStaggering and number of plates (SID_NumberOfLayers)");
			bRes=false;
		}
	}

	return bRes? StatusCode::SUCCESS:StatusCode::FAILURE;
}

StatusCode AFP_GeoModelTool::create()
{ 
    GeoModelExperiment * theExpt = nullptr;
    ATH_CHECK( detStore()->retrieve( theExpt, "ATLAS" ) );

	ATH_CHECK(checkPropertiesSettings());

    m_pGeometry=new AFP_Geometry(&m_CfgParams);
    m_pAFPDetectorFactory=std::make_unique<AFP_GeoModelFactory>(detStore().operator->(), m_pGeometry);

	if (m_detector==nullptr)
	{
		try
		{
			GeoPhysVol *world=&*theExpt->getPhysVol();
            m_pAFPDetectorFactory->create(world);
		}
		catch (std::bad_alloc const&)
		{
            ATH_MSG_FATAL("Could not create new DetectorNode!");
            return StatusCode::FAILURE;
        }

        // Register the DetectorNode instance with the Transient Detector Store
        theExpt->addManager(m_pAFPDetectorFactory->getDetectorManager());
        ATH_CHECK( detStore()->record(m_pAFPDetectorFactory->getDetectorManager(),
                                      m_pAFPDetectorFactory->getDetectorManager()->getName()) );
        return StatusCode::SUCCESS;
    }

    return StatusCode::FAILURE;
}
