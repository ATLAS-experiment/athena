/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "SCT_DetectorTool.h"

#include "SCT_DetectorFactory.h"
#include "SCT_DetectorFactoryLite.h"
#include "SCT_DataBase.h"
#include "SCT_Options.h"

#include "SCT_ReadoutGeometry/SCT_DetectorManager.h"

#include "GeoModelUtilities/GeoModelExperiment.h"
#include "GeoModelUtilities/DecodeVersionKey.h"
#include "RDBAccessSvc/IRDBRecord.h"
#include "RDBAccessSvc/IRDBRecordset.h"

using InDetDD::SCT_DetectorManager;
using InDetDD::SiDetectorManager;

SCT_DetectorTool::SCT_DetectorTool(const std::string& type,
                                   const std::string& name,
                                   const IInterface* parent)
  : GeoModelTool(type, name, parent)
{
}

// Create the Geometry via the factory corresponding to this tool

StatusCode
SCT_DetectorTool::create()
{

    // Get the detector configuration.
    ATH_CHECK(m_geoDbTagSvc.retrieve());

    ServiceHandle<IRDBAccessSvc> accessSvc(m_geoDbTagSvc->getParamSvcName(),name());
    ATH_CHECK(accessSvc.retrieve());

    // Print the SCT version tag:
    std::string detectorTag{""};
    std::string detectorNode{""};
    GeoModelIO::ReadGeoModel* sqliteReader  = m_geoDbTagSvc->getSqliteReader();

    if(!sqliteReader) {
        DecodeVersionKey versionKey{m_geoDbTagSvc.get(), "SCT"};
        ATH_MSG_INFO("Building SCT with Version Tag: " << versionKey.tag() << " at Node: " << versionKey.node());

        detectorTag = versionKey.tag();
        detectorNode = versionKey.node();

        std::string sctVersionTag{accessSvc->getChildTag("SCT", detectorTag, detectorNode)};

        ATH_MSG_INFO("SCT Version: " << sctVersionTag);
        if (sctVersionTag.empty()) {
	  ATH_MSG_INFO("No SCT Version. SCT will not be built.");
	  return StatusCode::SUCCESS;
        }
	else {
	  ATH_MSG_DEBUG("Keys for SCT Switches are "  << detectorTag  << "  " << detectorNode);
	}
    }
    std::string versionName;
    IRDBRecordset_ptr switchSet{accessSvc->getRecordsetPtr("SctSwitches", detectorTag, detectorNode)};
    const IRDBRecord* switches{(*switchSet)[0]};
    if (not switches->isFieldNull("COSMICLAYOUT")) {
      m_cosmic = switches->getInt("COSMICLAYOUT");
    }
    if (not switches->isFieldNull("VERSIONNAME")) {
      versionName = switches->getString("VERSIONNAME");
    }

    if (versionName.empty() && m_cosmic) {
      versionName = "SR1";
    }

    ATH_MSG_DEBUG("Creating the SCT");
    ATH_MSG_DEBUG("SCT Geometry Options: ");
    ATH_MSG_DEBUG(" Alignable:             " << (m_alignable.value() ? "true" : "false"));
    ATH_MSG_DEBUG(" CosmicLayout:          " << (m_cosmic ? "true" : "false"));
    ATH_MSG_DEBUG(" VersionName:           " << versionName);

    SCT_Options options;
    options.setAlignable(m_alignable.value());
    options.setDynamicAlignFolders(m_useDynamicAlignFolders.value());

    // Locate the top level experiment node
    GeoModelExperiment* theExpt{nullptr};
    ATH_CHECK(detStore()->retrieve(theExpt, "ATLAS"));

    // Retrieve the Geometry DB Interface
    ATH_CHECK(m_geometryDBSvc.retrieve());

    // Pass athena services to factory, etc
    m_athenaComps.setDetStore(detStore().operator->());
    m_athenaComps.setGeoDbTagSvc(&*m_geoDbTagSvc);
    m_athenaComps.setGeometryDBSvc(&*m_geometryDBSvc);
    m_athenaComps.setRDBAccessSvc(&*accessSvc);
    const SCT_ID* idHelper{nullptr};
    ATH_CHECK(detStore()->retrieve(idHelper, "SCT_ID"));
    m_athenaComps.setIdHelper(idHelper);

    GeoPhysVol* world{theExpt->getPhysVol()};

    // If we are using the SQLite reader, then we are not building the raw geometry but
    // just locating it and attaching to readout geometry and various other actions
    // taken in this factory.
    if (sqliteReader) {
      ATH_MSG_INFO("Building the geometry from the SQLite file");
      SCT_DetectorFactoryLite theSCT{sqliteReader, &m_athenaComps, options};
      theSCT.create(world);
      m_manager = theSCT.getDetectorManager();
    }
    else {
      SCT_DetectorFactory theSCT{&m_athenaComps, options};
      theSCT.create(world);
      m_manager = theSCT.getDetectorManager();
    }

    if (m_manager==nullptr) {
      ATH_MSG_FATAL("SCT_DetectorManager not created");
      return StatusCode::FAILURE;
    }

    ATH_MSG_DEBUG("Registering SCT_DetectorManager. ");
    ATH_CHECK(detStore()->record(m_manager, m_manager->getName()));
    theExpt->addManager(m_manager);

    // Create a symLink to the SiDetectorManager base class
    const SiDetectorManager* siDetManager{m_manager};
    ATH_CHECK(detStore()->symLink(m_manager, siDetManager));

    return StatusCode::SUCCESS;
}

StatusCode
SCT_DetectorTool::clear()
{
  SG::DataProxy* proxy{detStore()->proxy(ClassID_traits<SCT_DetectorManager>::ID(), m_manager->getName())};
  if (proxy) {
    proxy->reset();
    m_manager = nullptr;
  }
  return StatusCode::SUCCESS;
}

StatusCode
SCT_DetectorTool::align ATLAS_NOT_THREAD_SAFE (IOVSVC_CALLBACK_ARGS_P(I, keys))
//Not thread safe as the call m_manager->align will invalidateAllElements it holds
{
  if (m_manager==nullptr) {
    ATH_MSG_FATAL("Manager does not exist");
    return StatusCode::FAILURE;
  }
  if (m_alignable.value()) {
    return const_cast<InDetDD::SCT_DetectorManager*>(m_manager)->align(I, keys);
  } else {
    ATH_MSG_DEBUG("Alignment disabled. No alignments applied");
    return StatusCode::SUCCESS;
  }
}
