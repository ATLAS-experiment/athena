/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "ToroidDetectorTool.h"
#include <MuonReadoutGeometryR4/ToroidDetectorManager.h>
#include <GeoModelKernel/GeoVolumeCursor.h>
#include <GeoModelUtilities/GeoModelExperiment.h>

namespace MuonGMR4{
    ToroidDetectorTool::~ToroidDetectorTool() = default;

    StatusCode ToroidDetectorTool::create() {
        GeoModelExperiment *theExpt = nullptr;
        ATH_CHECK(detStore()->retrieve(theExpt, "ATLAS"));
        
        const std::vector<std::string>& nodeNames{m_treeTops};
        if (nodeNames.empty()) {
            return StatusCode::SUCCESS;
        }
        std::vector<PVConstLink> treeTops{};
 
        GeoVolumeCursor cursor{theExpt->getPhysVol()};

        while (!cursor.atEnd()) {
            std::string volName = cursor.getName();
            ATH_MSG_VERBOSE("Check whether \""<<volName<<"\" belongs to the muon world. ");
            if (std::ranges::find(nodeNames, volName) != nodeNames.end()) {
                treeTops.push_back(cursor.getVolume());
            }
            cursor.next();
        }
        ATH_MSG_DEBUG("Include "<<treeTops.size()<<"/"<<nodeNames.size()
                    <<" nodes into the toroid manager ("<<m_mgrName<<").");
        if (treeTops.empty()) {
            ATH_MSG_ERROR("No top node could be found");
            return StatusCode::FAILURE;

        }
        m_manager = new ToroidDetectorManager(m_mgrName);
        for (const auto& link : treeTops) {
            m_manager->addTreeTop(link);
        }
        ATH_CHECK(detStore()->record(m_manager, m_manager->getName()));
        theExpt->addManager(m_manager);

        return StatusCode::SUCCESS;
    } 

    StatusCode ToroidDetectorTool::clear() {
        if (!m_manager) {
            return StatusCode::SUCCESS;
        }
        SG::DataProxy *proxy = detStore()->proxy(ClassID_traits<ToroidDetectorManager>::ID(), 
                                                 m_manager->getName());
        if (proxy) {
            proxy->reset();
            m_manager = nullptr;
        }
        return StatusCode::SUCCESS;
    } 
        
}