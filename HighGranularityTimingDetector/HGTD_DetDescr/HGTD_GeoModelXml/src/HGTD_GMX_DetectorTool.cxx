/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "HGTD_GMX_DetectorTool.h"
#include "HGTD_GmxInterface.h"

#include <HGTD_ReadoutGeometry/HGTD_DetectorManager.h>

#include <GeoModelKernel/GeoPhysVol.h>
#include <GeoModelUtilities/GeoModelExperiment.h>
#include <SGTools/DataProxy.h>


HGTD_GMX_DetectorTool::HGTD_GMX_DetectorTool(const std::string &type,
                                             const std::string &name,
                                             const IInterface *parent)
    : GeoModelXmlTool(type, name, parent)
{
}


StatusCode HGTD_GMX_DetectorTool::create()
{
    // retrieve the common stuff
    ATH_CHECK(createBaseTool());

    const HGTD_ID *idHelper{};
    ATH_CHECK(detStore()->retrieve(idHelper, "HGTD_ID"));

    GeoModelExperiment *theExpt{};
    ATH_CHECK(detStore()->retrieve(theExpt, "ATLAS"));

    m_commonItems = std::make_unique<InDetDD::SiCommonItems>(idHelper);

    //
    // Check the availability
    //
    std::string node{"InnerDetector"};
    std::string table{"HGTDXDD"};

    GeoModelIO::ReadGeoModel* sqlreader = getSqliteReader();
    
    if(!sqlreader){
        if (!isAvailable(node, table)) {
            ATH_MSG_ERROR("No HGTD geometry found. HGTD can not be built.");
            return StatusCode::FAILURE;
        }
    }
    //
    // Create the detector manager
    //
    // The * converts a ConstPVLink to a ref to a GeoVPhysVol
    // The & takes the address of the GeoVPhysVol
    GeoPhysVol *world = &*theExpt->getPhysVol();
    auto *manager = new HGTD_DetectorManager(&*detStore());

    HGTD_GmxInterface gmxInterface(manager, m_commonItems.get());

    // Load the geometry, create the volume, 
    // node,table are the location in the DB to look for the clob
    // empty strings are the (optional) containing detector and envelope names
    // allowed to pass a null sqlreader ptr - it will be used to steer the source of the geometry
    const GeoVPhysVol* topVolume = createTopVolume(world, gmxInterface, node, table,"","",sqlreader);
    // if we are using SQLite inputs,
    if(sqlreader){
        bool useNewIdentifierScheme = idHelper->get_useNewIdentifierScheme();
        if (useNewIdentifierScheme){
            ATH_MSG_INFO("Building HGTD Readout Geometry from SQLite using "<<m_geoDbTagSvc->getParamSvcName());
            gmxInterface.buildReadoutGeometryFromSqlite(m_sqliteReadSvc.operator->(),sqlreader);
        } else {
            ATH_MSG_FATAL("SQLite workflow is unsupported for old HGTD identifier scheme!");
            return StatusCode::FAILURE;
        }
    }

    if (topVolume) { //see that a valid pointer is returned
        manager->addTreeTop(topVolume);
    } else {
        ATH_MSG_FATAL("Could not find the Top Volume!!!");
        return StatusCode::FAILURE;
    }

    // set the manager
    m_detManager = manager;

    // getName here will return whatever is passed as name to DetectorFactory - make this more explicit?
    ATH_CHECK(detStore()->record(m_detManager, m_detManager->getName()));
    theExpt->addManager(m_detManager);

    return StatusCode::SUCCESS;
}


StatusCode HGTD_GMX_DetectorTool::clear()
{
    // Release manager from the detector store
    SG::DataProxy* proxy = detStore()->proxy(ClassID_traits< HGTD_DetectorManager >::ID(), m_detManager->getName());
    if (proxy) {
        proxy->reset();
        m_detManager = nullptr;
    }

    return StatusCode::SUCCESS;
}
