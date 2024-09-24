/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTESTERTREE_EVENTINFOBRANCH_H
#define MUONTESTERTREE_EVENTINFOBRANCH_H
#include <MuonTesterTree/ScalarBranch.h>
#include <StoreGate/ReadHandleKey.h>
#include <StoreGate/ReadDecorHandleKey.h>
#include <xAODEventInfo/EventInfo.h>
#include <MuonTesterTree/MuonTesterBranch.h>
#include <MuonTesterTree/MuonTesterTree.h>
#include <atomic>

namespace MuonVal {
class EventInfoBranch : public MuonTesterBranch {
public:
    
    enum WriteOpts {
        /// Flag determining whether the branch is simulation
        isMC = 1,       
        /// Write pile-up information
        writePileUp = 1 << 1,
        /// Write the corrected pile-up
        writePRW = 1 << 2,
        /// Write the beamspot weight
        writeBeamSpot = 1 << 3,
        /// Write the trigger information
        writeTrigger = 1 << 4,
        /// Write each of the LHE weights
        writeLHE = 1<<5
    };
  
    EventInfoBranch(MuonTesterTree& tree, unsigned int write_mask,
                    const std::string& evtKey="EventInfo");

    bool fill(const EventContext& ctx) override final;
    bool init() override final;
    
    /// Specify the number of LHE variations that are available in the sample
    static void setNumLHE(unsigned int numLHE) ATLAS_THREAD_SAFE;
    static unsigned int getNumLHE();
  
private:
    static std::atomic<unsigned int> s_num_lhe ATLAS_THREAD_SAFE;
    /// Common access to the EventInfo
    SG::ReadHandleKey<xAOD::EventInfo> m_key{"EventInfo"};
    /// Access to the pile up weights if requested
    using EvtInfoDecor = SG::ReadDecorHandleKey<xAOD::EventInfo>;
    std::vector<EvtInfoDecor> m_prwKeys{};
    unsigned int m_writemask{0};

    
    ScalarBranch<unsigned long long>& m_evtNumber{parent().newScalar<unsigned long long>("eventNumber")};
    ScalarBranch<uint32_t>& m_runNumber{parent().newScalar<uint32_t>("runNumber")};

    ScalarBranch<uint32_t>& m_lbNumber{parent().newScalar<uint32_t>("lbNumber")};
    ScalarBranch<uint32_t>& m_bcid{parent().newScalar<uint32_t>("bcid")};
    
    /// Toggled by the write trigger flag
    ScalarBranch<uint32_t>& m_l1id{parent().newScalar<uint32_t>("l1id")};

    /// Toggled by the Write PileUp flag
    ScalarBranch<float>& m_average_mu{parent().newScalar<float>("average_mu")};
    ScalarBranch<float>& m_actual_mu{parent().newScalar<float>("actual_mu")};
    ///######################################################################
    ///                 Disabled if the job runs on data                    #
    ///######################################################################
    ScalarBranch<uint32_t>& m_mcChannel{parent().newScalar<uint32_t>("mcChannelNumber")};
    /// Removed from output if writeLHE is switched on 
    ScalarBranch<double>& m_weight{parent().newScalar<double>("mcEventWeight")};    
    std::map<unsigned int, std::shared_ptr<ScalarBranch<double>>> m_lhe_weights{};

    /// Branches toggled by the write prw Flag
    ScalarBranch<double>& m_prwWeight{parent().newScalar<double>("prwWeight")};
    ScalarBranch<uint32_t>& m_rnd_run{parent().newScalar<uint32_t>("randomRunNumber")};
    ScalarBranch<uint32_t>& m_rnd_lumi_block{parent().newScalar<uint32_t>("randomLumiBlockNumber")};
    ScalarBranch<double>& m_beamSpotWeight{parent().newScalar<double>("BeamSpotWeight")};
};
}
#endif
