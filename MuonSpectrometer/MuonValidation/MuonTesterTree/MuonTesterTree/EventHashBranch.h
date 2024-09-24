/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTESTERTREE_EVENTHASHBRANCH_H
#define MUONTESTERTREE_EVENTHASHBRANCH_H
#include <MuonTesterTree/ArrayBranch.h>
#include <xAODEventInfo/EventInfo.h>

/// Helper class to write a 2x64bit EventHash to the Tree
/// The hash can be used in the offline analysis to synchronize the
/// FriendTree reading
namespace MuonVal {
class EventHashBranch: public IMuonTesterBranch {
    public:
        ~EventHashBranch() = default;
        EventHashBranch(TTree* tree, const std::string& evtKey="EventInfo");
    
        bool fill(const EventContext& ctx) override final;
        bool init() override final;
        std::string name() const override final;
        std::vector<DataDependency> data_dependencies() override final;

        TTree* tree() override final;
        const TTree* tree() const override final;

        /// Returns true whether the current event is dumped to the N-tuple or not
        bool is_dumped(const EventContext& ctx) const;

    private:
        ArrayBranch<ULong64_t> m_cache;
        SG::ReadHandleKey<xAOD::EventInfo> m_evtKey{"EventInfo"};
        const EventContext* m_last_dump{nullptr};
};
}
#endif
