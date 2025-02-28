/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include "PixelDetectorTool.h"
#include "PixelDetectorFactory.h" 
#include "PixelDetectorFactoryLite.h" 
#include "PixelDetectorFactorySR1.h" 
#include "PixelDetectorFactoryDC2.h" 
#include "PixelGeometryManager.h" 
#include "PixelSwitches.h" 

#include "PixelReadoutGeometry/PixelDetectorManager.h" 
#include "ReadoutGeometryBase/InDetDD_Defs.h"
#include "DetDescrConditions/AlignableTransformContainer.h"
#include "PixelGeoModelAthenaComps.h"
#include "GeoModelUtilities/GeoModelExperiment.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GeoModelInterfaces/IGeoDbTagSvc.h"
#include "GeoModelUtilities/DecodeVersionKey.h"
#include "GeoModelRead/ReadGeoModel.h"

#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecord.h"
#include "RDBAccessSvc/IRDBRecordset.h"

#include "AthenaKernel/ClassID_traits.h"
#include "SGTools/DataProxy.h"

#include "CxxUtils/checker_macros.h"

using InDetDD::PixelDetectorManager; 
using InDetDD::SiDetectorManager; 

PixelDetectorTool::PixelDetectorTool( const std::string& type, const std::string& name, const IInterface* parent )
  : GeoModelTool( type, name, parent )
{
  declareProperty("Services",m_services);
  declareProperty("ServicesOnLadder",m_servicesOnLadder); ///JBdV
  declareProperty("Alignable", m_alignable);
  declareProperty("TweakIBLDist", m_tweakIBLDist);
  declareProperty("DC1Geometry",m_dc1Geometry);
  declareProperty("InitialLayout",m_initialLayout);
  declareProperty("DevVersion", m_devVersion);
  declareProperty("OverrideVersionName", m_overrideVersionName);
  declareProperty("useDynamicAlignFolders", m_useDynamicAlignFolders);
}

PixelDetectorTool::~PixelDetectorTool()
{
  delete m_athenaComps;
}


StatusCode PixelDetectorTool::initialize()
{
  if (!m_bcmTool.empty()) {
    ATH_CHECK( m_bcmTool.retrieve() );
  }
  if (!m_blmTool.empty()) {
    ATH_CHECK( m_blmTool.retrieve() );
  }
  return StatusCode::SUCCESS;
}

/**
 ** Create the Detector Node corresponding to this tool
 **/
StatusCode PixelDetectorTool::create()
{ 
  if (m_devVersion) ATH_MSG_WARNING("You are using a development version. There are no guarantees of stability");
   
  // Get the detector configuration.
  ServiceHandle<IGeoDbTagSvc> geoDbTagSvc("GeoDbTagSvc",name());
  ATH_CHECK(geoDbTagSvc.retrieve());

  ServiceHandle<IRDBAccessSvc> rdbAccessSvc(geoDbTagSvc->getParamSvcName(),name());
  ATH_CHECK(rdbAccessSvc.retrieve());


  GeoModelExperiment* theExpt = nullptr;
  ATH_CHECK(detStore()->retrieve(theExpt,"ATLAS"));

  GeoPhysVol *world=theExpt->getPhysVol();
  GeoModelIO::ReadGeoModel* sqliteReader = geoDbTagSvc->getSqliteReader();

  std::string detectorKey{""};
  std::string detectorNode{""};
  if(!sqliteReader) {
    DecodeVersionKey decodeVersion(geoDbTagSvc.operator->(), "Pixel");
    detectorKey = decodeVersion.tag();
    detectorNode = decodeVersion.node();
    if(decodeVersion.custom()) {
      ATH_MSG_WARNING("PixelDetectorTool:  Detector Information coming from a custom configuration!!" );
    } 
  }

  IRDBRecordset_ptr switchSet = rdbAccessSvc->getRecordsetPtr("PixelSwitches", detectorKey, detectorNode);
  const IRDBRecord  *switchTable = (*switchSet)[0];

  std::string versionName;
  std::string descrName="noDescr";

  m_dc1Geometry        = switchTable->getInt("DC1GEOMETRY");    
  m_initialLayout      = switchTable->getInt("INITIALLAYOUT");
  if (!switchTable->isFieldNull("VERSIONNAME")) {
    versionName        = switchTable->getString("VERSIONNAME");
  }
  if (!switchTable->isFieldNull("DESCRIPTION")) {
    descrName        = switchTable->getString("DESCRIPTION");
  }
  m_buildDBM        = switchTable->getInt("BUILDDBM");
  
  // Initialize switches
  PixelSwitches switches;
  
  switches.setServices(m_services);
  switches.setDC1Geometry(m_dc1Geometry);
  switches.setAlignable(m_alignable);
  switches.setInitialLayout(m_initialLayout);
  switches.setDBM(m_buildDBM); //DBM flag
  switches.setDynamicAlignFolders(m_useDynamicAlignFolders);
  
  //JBdV
  switches.setServicesOnLadder(m_servicesOnLadder);
  switches.setServices(m_services); //Overwrite there for the time being.
    
  const PixelID * idHelper = nullptr;
  ATH_CHECK(detStore()->retrieve(idHelper, "PixelID"));

  // Retrieve the Geometry DB Interface
  ATH_CHECK(m_geometryDBSvc.retrieve());

  // Pass athena services to factory, etc
  m_athenaComps = new PixelGeoModelAthenaComps;
  m_athenaComps->setDetStore(detStore().operator->());
  m_athenaComps->setGeoDbTagSvc(&*geoDbTagSvc);
  m_athenaComps->setRDBAccessSvc(&*rdbAccessSvc);
  m_athenaComps->setGeometryDBSvc(&*m_geometryDBSvc);
  m_athenaComps->setIdHelper(idHelper);

  // BCM Tool.
  if (!m_bcmTool.empty()) {
    if (!m_bcmTool.retrieve().isFailure()) {
      ATH_MSG_INFO("BCM_GeoModel tool retrieved: " << m_bcmTool );
    } else {
      ATH_MSG_INFO("Could not retrieve " << m_bcmTool << " -  BCM will not be built" );
    }
    m_athenaComps->setBCM(&*m_bcmTool);
  } 
  else {
    ATH_MSG_INFO("BCM not requested." );
  }
  
  // BLM Tool.
  if (!m_blmTool.empty()) {
    if (!m_blmTool.retrieve().isFailure()) {
      ATH_MSG_INFO("BLM_GeoModel tool retrieved: " << m_blmTool );
    } 
    else {
      ATH_MSG_INFO("Could not retrieve " << m_blmTool << " -  BLM will not be built" );
    }
    m_athenaComps->setBLM(&*m_blmTool);
  } 
  else {
    ATH_MSG_INFO("BLM not requested." );
  }
  
  // Service builder tool
  if (!m_serviceBuilderTool.empty()) {
    if (!m_serviceBuilderTool.retrieve().isFailure()) {
      ATH_MSG_INFO("Service builder tool retrieved: " << m_serviceBuilderTool );
      m_athenaComps->setServiceBuilderTool(&*m_serviceBuilderTool);
    } 
    else {
      ATH_MSG_ERROR("Could not retrieve " <<  m_serviceBuilderTool << ",  some services will not be built." );
    }
  } 
  else {
    if (versionName == "SLHC") { // TODO
      ATH_MSG_ERROR("Service builder tool not specified. Some services will not be built" );
    } 
    else {
      ATH_MSG_INFO("Service builder tool not specified." ); 
    }
  }

  if(sqliteReader) {
    // ---------------------- Build from SQLite file --------------------------
    ATH_MSG_INFO("Building the geometry from the SQLite file");
    
    ATH_MSG_DEBUG("Creating the Pixel ");
    ATH_MSG_DEBUG("Pixel Geometry Options:");
    ATH_MSG_DEBUG("  Services           = " << (m_services ? "true" : "false"));
    ATH_MSG_DEBUG("  Alignable          = " << (m_alignable ? "true" : "false"));
    ATH_MSG_DEBUG("  DC1Geometry        = " << (m_dc1Geometry ? "true" : "false"));
    ATH_MSG_DEBUG("  InitialLayout      = " << (m_initialLayout ? "true" : "false"));
    ATH_MSG_DEBUG("  VersioName         = " << versionName );

    if (versionName == "IBL") switches.setIBL();

    PixelDetectorFactoryLite thePixelFactory(sqliteReader,m_athenaComps,switches);
    thePixelFactory.create(world);
    m_manager=thePixelFactory.getDetectorManager();
  }
  else {   
    // ---------------------- Build from GeometryDB --------------------------    
    ATH_MSG_INFO("Building Pixel Detector with Version Tag: " << detectorKey
		 << " at Node: " << detectorNode);
		 
    ATH_MSG_INFO("Building the geometry in the standard way");

    // Print the version tag:
    std::string pixelVersionTag = rdbAccessSvc->getChildTag("Pixel", detectorKey, detectorNode);
    ATH_MSG_INFO("Pixel Version: " << pixelVersionTag );

    // Check if version is empty. If so, then the Pixel cannot be built. This may or may not be intentional.
    // We just issue an INFO message.
    if (pixelVersionTag.empty()) { 
      ATH_MSG_INFO("No Pixel Version. Pixel Detector will not be built." );
      return StatusCode::SUCCESS;
    }

    // Unless we are using custom pixel, the switch positions are going to
    // come from the database:
    
    ATH_MSG_DEBUG("PixelDetectorTool:  Detector Information coming from the database and job options IGNORED." );
    ATH_MSG_DEBUG("Keys for Pixel Switches are "  << detectorKey  << "  " << detectorNode );

    if (versionName.empty()) {
      if (m_dc1Geometry) {
	versionName = "DC1"; 
      } 
      else {
	versionName = "DC2"; 
      }
    }
    
    if (!m_overrideVersionName.empty()) {
      versionName = m_overrideVersionName;
      ATH_MSG_INFO("Overriding version name: " << versionName );
    }
    
    ATH_MSG_DEBUG("Creating the Pixel " );
    ATH_MSG_DEBUG("Pixel Geometry Options:" );
    ATH_MSG_DEBUG("  Services           = " << (m_services ? "true" : "false") );
    ATH_MSG_DEBUG("  Alignable          = " << (m_alignable ? "true" : "false"));
    ATH_MSG_DEBUG("  DC1Geometry        = " << (m_dc1Geometry ? "true" : "false"));
    ATH_MSG_DEBUG("  InitialLayout      = " << (m_initialLayout ? "true" : "false"));
    ATH_MSG_DEBUG("  VersioName         = " << versionName  );
    
    if (m_IBLParameterSvc.retrieve().isFailure()) {
      ATH_MSG_WARNING( "Could not retrieve IBLParameterSvc");
    }
    else {
      m_IBLParameterSvc->setBoolParameters(m_alignable,"alignable");
    }

    if (versionName == "IBL") switches.setIBL();            
    
    if (!m_devVersion) {
      
      if(versionName == "DC1" || versionName == "DC2") {	
	// DC1/DC2 version
	PixelDetectorFactoryDC2 thePixel(m_athenaComps, switches);
	thePixel.create(world);      
	m_manager  = thePixel.getDetectorManager();
      } 
      else if (versionName == "SR1") {
	// SR1. Same a DC3 but only 1 part (barrel, ec A or ec C) built
	PixelDetectorFactorySR1 thePixel(m_athenaComps, switches);
	thePixel.create(world);      
	m_manager  = thePixel.getDetectorManager();
      } 
      else {
	// DC3, SLHC, IBL
	PixelDetectorFactory thePixel(m_athenaComps, switches);
	if(descrName.compare("TrackingGeometry")!=0) {
	  thePixel.create(world);
	}
	else {
	  ATH_MSG_INFO("Pixel - TrackingGeometry tag - no geometry built" ); 
	}
	m_manager  = thePixel.getDetectorManager();
      }
      
    } 
    else {
      // DEVELOPMENT VERSIONS
      PixelDetectorFactory thePixel(m_athenaComps, switches);
      thePixel.create(world);      
      m_manager  = thePixel.getDetectorManager();
    }
  }
  
  // Register the manager to the Det Store    
  ATH_CHECK(detStore()->record(m_manager, m_manager->getName()));
  // Add the manager to the experiment 
  theExpt->addManager(m_manager);
    
  // Symlink the manager
  const SiDetectorManager * siDetManager = m_manager;
  ATH_CHECK(detStore()->symLink(m_manager, siDetManager));

  return StatusCode::SUCCESS;
}

StatusCode PixelDetectorTool::clear()
{
  SG::DataProxy* proxy = detStore()->proxy(ClassID_traits<InDetDD::PixelDetectorManager>::ID(),m_manager->getName());
  if(proxy) {
    proxy->reset();
    m_manager = nullptr;
  }
  return StatusCode::SUCCESS;
}
  
StatusCode 
PixelDetectorTool::align(IOVSVC_CALLBACK_ARGS_P(I,keys))
{
  if (!m_manager) { 
    ATH_MSG_WARNING("Manager does not exist" );
    return StatusCode::FAILURE;
  }    
  if (m_alignable) {     
    return m_manager->align(I,keys);
  } else {
    ATH_MSG_DEBUG("Alignment disabled. No alignments applied" );
    return StatusCode::SUCCESS;
  }
}
