// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#include "FPGATrackSimConfTools/FPGATrackSimEventSelectionTool.h"

FPGATrackSim::FPGATrackSimEventSelectionTool::FPGATrackSimEventSelectionTool(const std::string& algname, const std::string& name, const IInterface* ifc)
    : AthAlgTool(algname, name, ifc) {
}

StatusCode FPGATrackSim::FPGATrackSimEventSelectionTool::initialize() {
    // Initialize the event selection service
    ATH_CHECK(m_evtSel.retrieve());
 
    ATH_MSG_DEBUG("intialized");
    return StatusCode::SUCCESS;
}

bool FPGATrackSim::FPGATrackSimEventSelectionTool::selectEvent(const FPGATrackSimEventInputHeader& header) const {
    return m_evtSel->selectEvent(&header);
}