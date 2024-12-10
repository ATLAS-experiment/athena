/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "GeoDbTagSvc.h"
#include "RDBMaterialManager.h"
#include "GaudiKernel/ServiceHandle.h"

#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecordset.h"
#include "RDBAccessSvc/IRDBRecord.h"

GeoDbTagSvc::GeoDbTagSvc(const std::string& name,ISvcLocator* svc)
  : base_class(name,svc)
{
}

StatusCode GeoDbTagSvc::initialize()
{
  ATH_MSG_DEBUG("initialize()");
  return StatusCode::SUCCESS;
}

StatusCode GeoDbTagSvc::finalize()
{
  ATH_MSG_DEBUG("finalize()");
  return StatusCode::SUCCESS;
}

StatusCode GeoDbTagSvc::setupTags()
{
  ATH_MSG_DEBUG("setupTags()");

  if(getRdbAccess().isFailure()) {
    ATH_MSG_FATAL("IRDBAccessSvc not retrieved!");
    return StatusCode::FAILURE;
  }
  
  // Check if the Atlas version has already been set
  if(m_AtlasVersion.empty()) {
    ATH_MSG_FATAL("ATLAS tag not set!");
    return StatusCode::FAILURE;
  }

  // Get subsystem tags
  m_InDetVersion = (m_InDetVersionOverride.empty() 
            ?m_rdbAccesSvc->getChildTag("InnerDetector",m_AtlasVersion,"ATLAS") 
            : m_InDetVersionOverride);

  m_PixelVersion = (m_PixelVersionOverride.empty()
                    ?m_rdbAccesSvc->getChildTag("Pixel",m_InDetVersion,"InnerDetector")
            : m_PixelVersionOverride);

  m_SCT_Version = (m_SCT_VersionOverride.empty()
           ?m_rdbAccesSvc->getChildTag("SCT",m_InDetVersion,"InnerDetector")
           : m_SCT_VersionOverride);

  m_TRT_Version = (m_TRT_VersionOverride.empty()
           ?m_rdbAccesSvc->getChildTag("TRT",m_InDetVersion,"InnerDetector")
           : m_TRT_VersionOverride);

  m_LAr_Version = (m_LAr_VersionOverride.empty()
                   ?m_rdbAccesSvc->getChildTag("LAr",m_AtlasVersion,"ATLAS")
           : m_LAr_VersionOverride);

  m_TileVersion = (m_TileVersionOverride.empty()
                   ?m_rdbAccesSvc->getChildTag("TileCal",m_AtlasVersion,"ATLAS")
           : m_TileVersionOverride);

  m_MuonVersion = (m_MuonVersionOverride.empty()
                   ?m_rdbAccesSvc->getChildTag("MuonSpectrometer",m_AtlasVersion,"ATLAS")
           : m_MuonVersionOverride);

  m_CaloVersion = (m_CaloVersionOverride.empty()
                   ?m_rdbAccesSvc->getChildTag("Calorimeter",m_AtlasVersion,"ATLAS")
           : m_CaloVersionOverride);

  m_MagFieldVersion = (m_MagFieldVersionOverride.empty()
               ?m_rdbAccesSvc->getChildTag("MagneticField",m_AtlasVersion,"ATLAS")
               : m_MagFieldVersionOverride);

  m_CavernInfraVersion = (m_CavernInfraVersionOverride.empty()
              ?m_rdbAccesSvc->getChildTag("CavernInfra",m_AtlasVersion,"ATLAS")
              : m_CavernInfraVersionOverride);

  m_ForwardDetectorsVersion = (m_ForwardDetectorsVersionOverride.empty()
                   ?m_rdbAccesSvc->getChildTag("ForwardDetectors",m_AtlasVersion,"ATLAS")
                   : m_ForwardDetectorsVersionOverride);

  return StatusCode::SUCCESS;
}

StatusCode GeoDbTagSvc::setupConfig()
{
  ATH_MSG_DEBUG("setupConfig()");

  if(getRdbAccess().isFailure()) {
    ATH_MSG_FATAL("IRDBAccessSvc not retrieved!");
    return StatusCode::FAILURE;
  }

  // Check if the Atlas version has already been set
  if(!m_sqliteReader && m_AtlasVersion.empty()) {
    ATH_MSG_FATAL("ATLAS tag not set!");
    return StatusCode::FAILURE;
  }

  // Retrieve geometry config information (RUN1, RUN2, etc...)
  IRDBRecordset_ptr atlasCommonRec =m_rdbAccesSvc->getRecordsetPtr("AtlasCommon",m_AtlasVersion,"ATLAS");
  if(atlasCommonRec->size()==0) {
    m_geoConfig = GeoModel::GEO_RUN1;
  }
  else {
    std::string configVal = (*atlasCommonRec)[0]->getString("CONFIG");
    if(configVal=="RUN1")
      m_geoConfig = GeoModel::GEO_RUN1;
    else if(configVal=="RUN2")
      m_geoConfig = GeoModel::GEO_RUN2;
    else if(configVal=="RUN3")
      m_geoConfig = GeoModel::GEO_RUN3;
    else if(configVal=="RUN4")
      m_geoConfig = GeoModel::GEO_RUN4;
    else if(configVal=="TESTBEAM")
      m_geoConfig = GeoModel::GEO_TESTBEAM;
    else {
      ATH_MSG_FATAL("Unexpected value for geometry config read from the database: " << configVal);
      return StatusCode::FAILURE;
    }
  }

  return StatusCode::SUCCESS;
}

StatusCode GeoDbTagSvc::getRdbAccess()
{
  if(!m_rdbAccesSvc) {
    SmartIF<IRDBAccessSvc> rdbAccessSvc{Gaudi::svcLocator()->service(m_paramSvcName)};
    if(!rdbAccessSvc.isValid()) return StatusCode::FAILURE;
    m_rdbAccesSvc = rdbAccessSvc.operator->();
  }
  return StatusCode::SUCCESS;
}
