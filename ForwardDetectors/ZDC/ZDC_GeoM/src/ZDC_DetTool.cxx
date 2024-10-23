/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "ZDC_DetTool.h"
#include "ZDC_DetFactory.h" 
#include "ZDC_DetManager.h" 
#include "GeoModelUtilities/GeoModelExperiment.h"
#include "GaudiKernel/IService.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GeoModelInterfaces/IGeoDbTagSvc.h"
#include "StoreGate/StoreGateSvc.h"
#include "AthenaKernel/getMessageSvc.h"

ZDC_DetTool::ZDC_DetTool(const std::string& type, const std::string& name, const IInterface* parent)
  : GeoModelTool(type, name, parent)
{

  if (msgLevel(MSG::DEBUG))
    msg(MSG::DEBUG) << "INSIDE CONSTRUCTOR OF DETTOOL"                            << endmsg
		    << "INSIDE CONSTRUCTOR OF DETTOOL string& type "      << type << endmsg
		    << "INSIDE CONSTRUCTOR OF DETTOOL std::string& name " << name << endmsg;
}

ZDC_DetTool::~ZDC_DetTool()
{
  // This will need to be modified once we register the Toy DetectorNode in the Transient Detector Store
  
  if (nullptr != m_detector) {

    delete m_detector;
    m_detector = nullptr;
  }
}

StatusCode ZDC_DetTool::create()
{ 

  // Locate the top level experiment node  
  GeoModelExperiment* theExpt = nullptr;
  
  ATH_CHECK( detStore()->retrieve(theExpt, "ATLAS") );

  ZDC_DetFactory theZDCFactory(detStore().operator->());

  ServiceHandle<IGeoDbTagSvc> geoDbTag("GeoDbTagSvc", name());
  ATH_CHECK( geoDbTag.retrieve() );

  GeoModel::GeoConfig geoConfig = geoDbTag->geoConfig();
  
  //Set the geometry configuration
  if(geoConfig==GeoModel::GEO_RUN2){ 
    ATH_MSG_INFO("Initializing ZDC geometry for PbPb2015");
    theZDCFactory.initializePbPb2015();
  }else if(geoConfig==GeoModel::GEO_RUN3){ 
    ATH_MSG_INFO("Initializing ZDC geometry for PbPb2023");
    theZDCFactory.initializePbPb2023();
  }else if(geoConfig==GeoModel::GEO_RUN4){
    ATH_MSG_ERROR("No ZDC geometry defined for RUN4");
  }

  if (nullptr == m_detector) { // Create the ZDCDetectorNode instance
    
    try { 
      // This strange way of casting is to avoid an utterly brain damaged compiler warning.
      GeoPhysVol* world = &*theExpt->getPhysVol();
      theZDCFactory.create(world);  
    } 
    catch (const std::bad_alloc&) {
      
      ATH_MSG_FATAL("Could not create new ZDC DetectorNode!");
      return StatusCode::FAILURE; 
    }
    
    // Register the ZDC DetectorNode instance with the Transient Detector Store
    theExpt->addManager(theZDCFactory.getDetectorManager());
    ATH_CHECK( detStore()->record(theZDCFactory.getDetectorManager(),theZDCFactory.getDetectorManager()->getName()) );
  }
  
  return StatusCode::FAILURE;
}
