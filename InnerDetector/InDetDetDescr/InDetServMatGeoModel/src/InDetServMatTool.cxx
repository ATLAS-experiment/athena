/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetServMatTool.h"
#include "InDetServMatFactory_Lite.h"
#include "InDetServMatFactory.h"

#include "GeoModelUtilities/GeoModelExperiment.h"
#include "GeoModelUtilities/DecodeVersionKey.h"
#include "StoreGate/StoreGateSvc.h"
#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecord.h"
#include "RDBAccessSvc/IRDBRecordset.h"
#include "AthenaKernel/errorcheck.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"

#include "AthenaKernel/ClassID_traits.h"
#include "SGTools/DataProxy.h"

/**
 ** Constructor(s)
 **/
InDetServMatTool::InDetServMatTool( const std::string& type, const std::string& name, const IInterface* parent )
  : GeoModelTool( type, name, parent )
{
}

/**
 ** Create the Detector Node corresponding to this tool
 **/
StatusCode InDetServMatTool::create()
{ 

  // 
  // Locate the top level experiment node 
  // 
  GeoModelExperiment * theExpt{nullptr};
  ATH_CHECK(detStore()->retrieve(theExpt,"ATLAS"));
  
  // Get the detector configuration.
  ATH_CHECK(m_geoDbTagSvc.retrieve());

  ServiceHandle<IRDBAccessSvc> accessSvc(m_geoDbTagSvc->getParamSvcName(),name());
  ATH_CHECK(accessSvc.retrieve());

  GeoPhysVol *world=theExpt->getPhysVol();

  GeoModelIO::ReadGeoModel* sqliteReader  = m_geoDbTagSvc->getSqliteReader();

  if (sqliteReader) {
    InDetServMatFactory_Lite factoryLite;;
    factoryLite.create(world);
    m_manager = factoryLite.getDetectorManager();
  }
  else {
    DecodeVersionKey versionKey(m_geoDbTagSvc.get(), "InnerDetector");
    
    std::string versionTag = accessSvc->getChildTag("InDetServices", versionKey.tag(), versionKey.node());
    ATH_MSG_DEBUG("versionTag=" << versionTag << " %%%");
    
    // If versionTag is NULL then don't build.
    if (versionTag.empty()) { 
      ATH_MSG_INFO("No InDetService Version. InDetService will not be built.");
      ATH_MSG_DEBUG("InnerDetector Version Tag: " << versionKey.tag() << " at Node: " << versionKey.node());
      return StatusCode::SUCCESS;
    } 
    
    ATH_MSG_DEBUG("Keys for InDetServMat Switches are "  << versionKey.tag()  << "  " << versionKey.node());
    
    std::string versionName{"CSC"};
    if (!m_overrideVersionName.empty()) {
      versionName = m_overrideVersionName;
      ATH_MSG_INFO("Overriding version name: " << versionName);
    }
    ATH_MSG_INFO("Building Inner Detector Service Material. Version: " << versionName);
    
    // Retrieve the Geometry DB Interface
    ATH_CHECK(m_geometryDBSvc.retrieve());
    
    // Pass athena services to factory, etc
    m_athenaComps.setDetStore(detStore().get());
    m_athenaComps.setGeoDbTagSvc(m_geoDbTagSvc.get());
    m_athenaComps.setRDBAccessSvc(accessSvc.get());
    m_athenaComps.setGeometryDBSvc(m_geometryDBSvc.get());
    
    // Retrieve builder tool (SLHC only)
    if (versionName == "SLHC") {
      if (!m_builderTool.empty()) {
	if(m_builderTool.retrieve().isFailure()) {
	  ATH_MSG_WARNING("Could not retrieve " <<  m_builderTool << ",  some services will not be built.");
	}
	else {
	  ATH_MSG_INFO("Service builder tool retrieved: " << m_builderTool);
	  m_athenaComps.setBuilderTool(&*m_builderTool);
	} 
      }
      else {
	// This will become an error once the tool is ready.
	ATH_MSG_INFO("Service builder tool not specified.");
      }
    }
    
    if (versionName == "CSC") {
      ATH_MSG_DEBUG("InDetServMat Factory CSC");
      InDetServMatFactory theIDSM(&m_athenaComps);
      theIDSM.create(world);
      m_manager=theIDSM.getDetectorManager();
    } else {
      // Unrecognized name.
      ATH_MSG_FATAL("Unrecognized VersionName: " << versionName);
      return StatusCode::FAILURE;
    }
  }

  if (m_manager) {
    theExpt->addManager(m_manager);
    ATH_CHECK(detStore()->record (m_manager, m_manager->getName()));
  }
  else {
    ATH_MSG_FATAL("Could not create InDetServMatManager!");
    return StatusCode::FAILURE;     
  }
  
  return StatusCode::SUCCESS;
}

StatusCode InDetServMatTool::clear()
{
  SG::DataProxy* proxy = detStore()->proxy(ClassID_traits<InDetDD::InDetServMatManager>::ID(),m_manager->getName());
  if(proxy) {
    proxy->reset();
    m_manager = nullptr;
  }
  return StatusCode::SUCCESS;
}
