
/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonTesterTree/EventHashBranch.h>
#include <StoreGate/ReadHandle.h>
namespace MuonVal {

EventHashBranch::EventHashBranch(TTree* tree, const std::string& evtKey):
    m_cache{tree, "CommonEventHash", 2},
    m_evtKey{evtKey} {}
bool EventHashBranch::init() {return m_cache.init() && m_evtKey.initialize();}
std::string EventHashBranch::name() const {return m_cache.name();}
std::vector<EventHashBranch::DataDependency> EventHashBranch::data_dependencies() {
    return {&m_evtKey};
}
TTree* EventHashBranch::tree() {return m_cache.tree();}
const TTree*EventHashBranch::tree() const {return m_cache.tree();}
bool EventHashBranch::fill(const EventContext& ctx) {
    SG::ReadHandle<xAOD::EventInfo> evtInfo{m_evtKey, ctx};
    if (!evtInfo.isValid()){
       MsgStream log(Athena::getMessageSvc(), name());
        log << MSG::ERROR << "Could not retrieve the EventInfo " << m_evtKey.fullKey() << endmsg;
        return false;
    }
    m_cache[0] = evtInfo->eventNumber();
    if (evtInfo->eventType(xAOD::EventInfo::IS_SIMULATION)) {
        ULong64_t dsid = evtInfo->mcChannelNumber();
        dsid = ( (dsid<<32) | evtInfo->runNumber());
        m_cache[1] = dsid;
    } else m_cache[1] = evtInfo->runNumber();
    m_last_dump = &ctx;
    return m_cache.fill(ctx);
}
bool EventHashBranch::is_dumped(const EventContext& ctx ) const{
    return &ctx == m_last_dump;
}
}