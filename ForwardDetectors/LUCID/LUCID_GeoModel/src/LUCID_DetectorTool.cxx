/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "LUCID_DetectorTool.h"
#include "LUCID_DetectorFactory.h" 
#include "LUCID_GeoModel/LUCID_DetectorManager.h" 

#include "GeoModelInterfaces/IGeoDbTagSvc.h"
#include "GeoModelUtilities/GeoModelExperiment.h"
#include "GaudiKernel/IService.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/MsgStream.h"
#include "GeoModelUtilities/GeoBorderSurfaceContainer.h"

#include "StoreGate/StoreGateSvc.h"

#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "AthenaKernel/ClassID_traits.h"
#include "SGTools/DataProxy.h"

LUCID_DetectorTool::LUCID_DetectorTool(const std::string& type, 
				       const std::string& name, 
				       const IInterface* parent): 
  GeoModelTool(type, name, parent),
  m_manager(nullptr)
{
}

LUCID_DetectorTool::~LUCID_DetectorTool() {}

StatusCode LUCID_DetectorTool::create() { 
  
  MsgStream log(msgSvc(), name()); 
  
  ATH_MSG_INFO("Building LUCID geometry");
  
  ServiceHandle<IGeoDbTagSvc> geoDbTag("GeoDbTagSvc", name());
  ATH_CHECK( geoDbTag.retrieve() );

  ServiceHandle<IRDBAccessSvc> raccess("RDBAccessSvc", name());
  ATH_CHECK( raccess.retrieve() );

  const std::string AtlasVersion = geoDbTag->atlasVersion();
  const std::string LucidVersion = raccess->getChildTag("LUCID",AtlasVersion,"ATLAS");

  if(LucidVersion.empty()) {
    ATH_MSG_DEBUG("LUCID is not part of the selected ATLAS geometry. Skipping");
    return StatusCode::SUCCESS;
  }

  GeoModelExperiment* theExpt; 
  ATH_CHECK( detStore()->retrieve(theExpt, "ATLAS") );

  if(nullptr == m_detector) {

    GeoPhysVol* world = &*theExpt->getPhysVol();
        
    LUCID_DetectorFactory theLUCID_Factory(detStore().get(),raccess.get());

    theLUCID_Factory.create(world);
    m_manager = theLUCID_Factory.getDetectorManager();
    theExpt->addManager(m_manager);

    ATH_CHECK( detStore()->record(m_manager, m_manager->getName()) );
    return StatusCode::SUCCESS;
  }
  
  return StatusCode::FAILURE;
}

StatusCode LUCID_DetectorTool::clear() {

  SG::DataProxy* proxy = detStore()->proxy(ClassID_traits<LUCID_DetectorManager>::ID(),m_manager->getName());

  if(proxy) {
    proxy->reset();
    m_manager = nullptr;
  }
  
  proxy = detStore()->proxy(ClassID_traits<GeoBorderSurfaceContainer>::ID(), "LUCID", false);
  
  if(proxy) {
    proxy->reset();
  }

  return StatusCode::SUCCESS;
}
