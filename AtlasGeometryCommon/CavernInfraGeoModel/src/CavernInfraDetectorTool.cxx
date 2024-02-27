/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "CavernInfraDetectorTool.h"
#include "CavernInfraDetectorFactory.h" 
#include "CavernInfraDetectorManager.h" 

#include "GeoModelInterfaces/IGeoDbTagSvc.h"
#include "GeoModelUtilities/GeoModelExperiment.h"
#include "RDBAccessSvc/IRDBAccessSvc.h"


CavernInfraDetectorTool::CavernInfraDetectorTool( const std::string& type, 
						  const std::string& name,
						  const IInterface* parent )
  : GeoModelTool( type, name, parent )
{
}

StatusCode CavernInfraDetectorTool::create()
{ 
  ServiceHandle<IGeoDbTagSvc> geoDbTag("GeoDbTagSvc", name());
  ATH_CHECK(geoDbTag.retrieve());

  std::string cavernInfraVersion = geoDbTag->cavernInfraVersion();
  if(cavernInfraVersion.empty()) {
    ATH_MSG_INFO("No Cavern Infra version for the given configuration. Skip building CavernInfraGeoModel");
    return StatusCode::SUCCESS;
  }
  else {
    ATH_MSG_INFO("Building Cavern geometry version " << cavernInfraVersion);
  }

  std::string versionNode = "CavernInfra";

  GeoModelExperiment* theExpt = nullptr;
  ATH_CHECK(detStore()->retrieve(theExpt,"ATLAS"));
 
  GeoPhysVol *world=theExpt->getPhysVol();
  ServiceHandle<IRDBAccessSvc> raccess("RDBAccessSvc",name());
  ATH_CHECK(raccess.retrieve());

  if(!m_manager) {
    CavernInfraDetectorFactory theCavernInfraFactory(detStore(),raccess);
    theCavernInfraFactory.setTagNode(cavernInfraVersion,versionNode);
    theCavernInfraFactory.create(world);
    m_manager = theCavernInfraFactory.getDetectorManager();
  }

  if(m_manager) {
    theExpt->addManager(m_manager);
    ATH_CHECK(detStore()->record(m_manager,m_manager->getName()));
  }
  else {
    ATH_MSG_ERROR("ERROR. Failed to build Cavern Version " << cavernInfraVersion);
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

StatusCode CavernInfraDetectorTool::clear()
{
  SG::DataProxy* proxy = detStore()->proxy(ClassID_traits<CavernInfraDetectorManager>::ID(),m_manager->getName());
  if(proxy) {
    proxy->reset();
    m_manager = nullptr;
  }
  return StatusCode::SUCCESS;
}

