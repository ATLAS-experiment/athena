/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonTesterTree/EventIDBranch.h"

namespace MuonVal{
    EventIDBranch::EventIDBranch(MuonTesterTree& parent, const std::string& objName):
        MuonTesterBranch{parent, objName} {}


    void EventIDBranch::operator=(const EventIDBase& eventID){
        set(eventID);
    }
    void EventIDBranch::set(const EventIDBase& eventID){
        m_lumiBlock = eventID.lumi_block();
        m_runNumber = eventID.run_number();
        m_eventNumber = eventID.event_number();
        m_timeStamp = eventID.time_stamp();
    }
    bool EventIDBranch::fill(const EventContext&) {
        return true;
    }
    bool EventIDBranch::init() {
        return true;
    }
}