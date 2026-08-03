/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "BeamPipeDetectorTool.h"
#include "BeamPipeDetectorFactory.h"
#include "BeamPipeDetectorFactory_Lite.h"
#include "BeamPipeGeoModel/BeamPipeDetectorManager.h"

#include "GeoModelInterfaces/IGeoDbTagSvc.h"
#include "GeoModelUtilities/GeoModelExperiment.h"
#include "GaudiKernel/IService.h"
#include "GaudiKernel/ISvcLocator.h"

#include "StoreGate/StoreGateSvc.h"

#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "AthenaKernel/ClassID_traits.h"
#include "SGTools/DataProxy.h"

#include "GeoModelKernel/GeoVolumeCursor.h"

BeamPipeDetectorTool::BeamPipeDetectorTool( const std::string& type,
					    const std::string& name,
					    const IInterface* parent ):
  GeoModelTool(type,name,parent)
{
}

BeamPipeDetectorTool::~BeamPipeDetectorTool() = default;


StatusCode BeamPipeDetectorTool::create()
{
  ATH_MSG_INFO("Building Beam Pipe");

  SmartIF<IGeoDbTagSvc> geoDbTag{Gaudi::svcLocator()->service("GeoDbTagSvc")};
  ATH_CHECK(geoDbTag.isValid());

  std::string atlasVersion = geoDbTag->atlasVersion();
  std::string versionNode = "ATLAS";

  GeoModelExperiment* theExpt{nullptr};
  ATH_CHECK(detStore()->retrieve(theExpt, "ATLAS"));

  GeoPhysVol* world=theExpt->getPhysVol();

  ServiceHandle<IRDBAccessSvc> accessSvc(geoDbTag->getParamSvcName(),name());
  ATH_CHECK(accessSvc.retrieve());

  GeoModelIO::ReadGeoModel* sqliteReader  = geoDbTag->getSqliteReader();
  if (sqliteReader) {
    BeamPipeDetectorFactory_Lite theBeamPipeFactory;
    theBeamPipeFactory.create(world);
    m_manager = theBeamPipeFactory.getDetectorManager();
  }
  else {
    // Check we have the beampipe and print its version
    // Print the  version tag:
    std::string beampipeVersionTag;
    beampipeVersionTag = accessSvc->getChildTag("BeamPipe", atlasVersion,versionNode);
    ATH_MSG_DEBUG("Beampipe Version: " << beampipeVersionTag);

    if (beampipeVersionTag.empty()) {
      ATH_MSG_INFO("No BeamPipe Version. Beam pipe will not be built.");
    }
    else {
      BeamPipeDetectorFactory theBeamPipeFactory(detStore().operator->(),accessSvc.operator->());
      theBeamPipeFactory.setTagNode(std::move(atlasVersion),std::move(versionNode),m_mode);
      theBeamPipeFactory.create(world);
      
      m_manager = theBeamPipeFactory.getDetectorManager();
    }
  }

  if (m_manager) {
    theExpt->addManager(m_manager);
    ATH_CHECK(detStore()->record(m_manager,m_manager->getName()));
    return StatusCode::SUCCESS;
  }

  return StatusCode::FAILURE;
}

StatusCode BeamPipeDetectorTool::clear()
{
  SG::DataProxy* proxy = detStore()->proxy(ClassID_traits<BeamPipeDetectorManager>::ID(),m_manager->getName());
  if(proxy) {
    proxy->reset();
    m_manager = nullptr;
  }
  return StatusCode::SUCCESS;
}
