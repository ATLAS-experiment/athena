// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

/**
 * @file FPGATrackSimRoadUnionTool.cxx
 * @author Riley Xu - riley.xu@cern.ch
 * @date November 20th, 2020
 * @brief See header file.
 */


#include "FPGATrackSimRoadUnionTool.h"


FPGATrackSimRoadUnionTool::FPGATrackSimRoadUnionTool(const std::string& algname, const std::string &name, const IInterface *ifc) :
    base_class(algname, name, ifc),
    m_tools(this)
{
    declareInterface<IFPGATrackSimRoadFinderTool>(this);
    declareProperty("tools", m_tools, "Array of FPGATrackSimRoadFinderTools");
}


StatusCode FPGATrackSimRoadUnionTool::initialize()
{
    // Retrieve
    ATH_MSG_INFO("Using " << m_tools.size() << " tools");
    ATH_CHECK(m_tools.retrieve());

    if (m_tools.empty()) {
      ATH_MSG_FATAL("initialize() Tool list empty");
      return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
}


StatusCode FPGATrackSimRoadUnionTool::getRoads(const std::vector<std::shared_ptr<const FPGATrackSimHit>> & hits, std::vector<std::shared_ptr<const FPGATrackSimRoad>> & roads) 
{
    roads.clear();
    for (auto & tool : m_tools)
    {
        std::vector<std::shared_ptr<const FPGATrackSimRoad>> r;
        ATH_CHECK(tool->getRoads(hits, r));
        roads.insert(roads.end(), std::make_move_iterator(r.begin()), std::make_move_iterator(r.end()));
    }

    return StatusCode::SUCCESS;
}
