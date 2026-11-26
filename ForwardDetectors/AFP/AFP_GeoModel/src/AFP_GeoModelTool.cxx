/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "GeoModelUtilities/GeoModelExperiment.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "AthenaPoolUtilities/AthenaAttributeList.h"

#include "AFP_GeoModelTool.h"
#include "AFP_GeoModelFactory.h"
#include "AFP_GeoModelManager.h"

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

	m_CfgParams.clear();
	for(int i=0; i<4; ++i){
		m_CfgParams.vecRPotFloorDistance[i]=m_rpotFloorDistance[i];
		m_CfgParams.vecRPotYPos[i]=m_rpotYPos[i];
		m_CfgParams.vecStatNominalZPos[i]=m_statNominalZPos[i];
	}
	for(eAFPStation eStation : {EAS_AFP00, EAS_AFP01, EAS_AFP02, EAS_AFP03}){
		m_CfgParams.sidcfg[eStation].bAddVacuumSensors=m_addVacuumSensors;
	}

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
