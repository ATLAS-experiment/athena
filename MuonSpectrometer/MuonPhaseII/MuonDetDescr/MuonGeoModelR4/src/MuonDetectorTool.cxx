/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonGeoModelR4/MuonDetectorTool.h"

#include <GeoModelInterfaces/IGeoModelSvc.h>
#include <GeoModelKernel/GeoPerfUtils.h>
#include <GeoModelKernel/GeoPhysVol.h>
#include <GeoModelKernel/GeoVolumeCursor.h>
#include <GeoModelRead/ReadGeoModel.h>
#include <GeoModelUtilities/GeoModelExperiment.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>


namespace MuonGMR4 {
MuonDetectorTool::MuonDetectorTool(const std::string &type,
                                   const std::string &name,
                                   const IInterface *parent)
    : GeoModelTool(type, name, parent) {
    declareInterface<IGeoModelTool>(this);
}

StatusCode MuonDetectorTool::initialize() {
    ATH_MSG_INFO("Initializing ...");
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_detTechTools.retrieve());
    ATH_CHECK(m_geoDbTagSvc.retrieve());
    return StatusCode::SUCCESS;
}

MuonDetectorTool::~MuonDetectorTool() = default;
/**
 ** Create the Detector Node corresponding to this tool
 **/
StatusCode MuonDetectorTool::create() {

    m_manager = new MuonDetectorManager();

    GeoModelExperiment *theExpt = nullptr;
    ATH_CHECK(detStore()->retrieve(theExpt, "ATLAS"));
    GeoPhysVol *world = theExpt->getPhysVol();

    for (auto &readOutTool : m_detTechTools) {
        const unsigned memBefore = GeoPerfUtils::getMem();
        ATH_CHECK(readOutTool->buildReadOutElements(*m_manager));
        const unsigned memAfter = GeoPerfUtils::getMem();
        ATH_MSG_INFO("Building of "<<readOutTool->name()<<" consumed "<<(memAfter - memBefore) / 1024<<" MB memory");
    }
    GeoVolumeCursor cursor(world);
    while (!cursor.atEnd()) {
        std::string volName = cursor.getName();
        ATH_MSG_VERBOSE("Check whether "<<volName<<" belongs to the muon world. ");
        if (std::find(m_treeTopNodes.value().begin(), 
                      m_treeTopNodes.value().end(),volName) != m_treeTopNodes.value().end()) {
            m_manager->addTreeTop(cursor.getVolume());
        }
        cursor.next();
    }
    ATH_CHECK(detStore()->record(m_manager, m_manager->getName()));
    ATH_CHECK(detStore()->retrieve(theExpt, "ATLAS"));
    theExpt->addManager(m_manager);

    return StatusCode::SUCCESS;
}

StatusCode MuonDetectorTool::clear() {
    SG::DataProxy *proxy = detStore()->proxy(
        ClassID_traits<MuonDetectorManager>::ID(), m_manager->getName());
    if (proxy) {
        proxy->reset();
        m_manager = nullptr;
    }
    return StatusCode::SUCCESS;
}

}  // namespace MuonGMR4
