/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonTesterTree/EventInfoBranch.h>
#include <StoreGate/ReadHandle.h>
#include <MuonTesterTree/throwExcept.h>
#include <stdexcept>
namespace {
     static const SG::AuxElement::ConstAccessor<unsigned int> acc_Random("RandomRunNumber");
     static const SG::AuxElement::ConstAccessor<unsigned int> acc_LumiBlock("RandomLumiBlockNumber");
     static const SG::AuxElement::ConstAccessor<float> acc_PuW("PileupWeight");
}
namespace MuonVal {
std::atomic<unsigned int> EventInfoBranch::s_num_lhe = 0;
void EventInfoBranch::setNumLHE(unsigned int n) { s_num_lhe = n;}
unsigned int EventInfoBranch::getNumLHE() { return s_num_lhe;}
EventInfoBranch::EventInfoBranch(MuonTesterTree& tree, unsigned int write_mask):
    MuonTesterBranch{tree, " event info "},    
    m_writemask{write_mask} {
    if (m_writemask & WriteOpts::isMC) {
        if (m_writemask & WriteOpts::writeLHE) {
            for (unsigned int lhe = 1; lhe < s_num_lhe ; ++lhe ) {
                std::shared_ptr<ScalarBranch<double>>& new_br = m_lhe_weights[lhe];                
                new_br = std::make_shared<ScalarBranch<double>>(tree.tree(),  "mcEventWeight_LHE_" + std::to_string(lhe),0.);
                if (!tree.addBranch(new_br)) {
                  THROW_EXCEPTION("EventInfoBranch: Failed to create Scalar branch in c'tor");
                }
            }            
        }
    } else {
         tree.disableBranch(m_mcChannel.name());
         tree.disableBranch(m_weight.name()); 
         m_writemask&= ~(WriteOpts::writePRW | WriteOpts::writeBeamSpot);   
    }
    
    if (m_writemask & WriteOpts::writePRW) {
        m_prwKeys.emplace_back(m_key.key() + "." + SG::AuxTypeRegistry::instance().getName(acc_Random.auxid()));
        m_prwKeys.emplace_back(m_key.key() + "." + SG::AuxTypeRegistry::instance().getName(acc_LumiBlock.auxid()));
        m_prwKeys.emplace_back(m_key.key() + "." + SG::AuxTypeRegistry::instance().getName(acc_PuW.auxid()));
     } else {
        tree.disableBranch(m_prwWeight.name());
        tree.disableBranch(m_rnd_run.name());
        tree.disableBranch(m_rnd_lumi_block.name());
     }
     if (!(m_writemask & WriteOpts::writeBeamSpot)) tree.disableBranch(m_beamSpotWeight.name());
     if (!(m_writemask & WriteOpts::writePileUp)) {
        tree.disableBranch(m_average_mu.name());
        tree.disableBranch(m_actual_mu.name());
     }
     if (!(m_writemask & WriteOpts::writeTrigger)) tree.disableBranch(m_l1id.name());
}
   


bool EventInfoBranch::fill(const EventContext& ctx) {
    SG::ReadHandle<xAOD::EventInfo> evt_info{m_key, ctx};
    if (!evt_info.isValid()) {
        ATH_MSG_ERROR("Could not retrieve the EventInfo " << m_key.fullKey());
        return false;
    }

    m_evtNumber = evt_info->eventNumber();
    m_runNumber = evt_info->runNumber();
    m_lbNumber = evt_info->lumiBlock();
    m_bcid = evt_info->bcid();
    
    m_average_mu = evt_info->averageInteractionsPerCrossing();
    m_actual_mu = evt_info->actualInteractionsPerCrossing();
    if (m_writemask & WriteOpts::writeTrigger) {
        m_l1id = evt_info->extendedLevel1ID();
    }    
    if (m_writemask & WriteOpts::isMC) {
        m_mcChannel = evt_info->mcChannelNumber();
        m_weight = evt_info->mcEventWeight(0);
        const unsigned int n_weights = evt_info->mcEventWeights().size();
        for (auto & [lhe_idx, lhe_branch] : m_lhe_weights) { 
            (*lhe_branch) = lhe_idx < n_weights ? evt_info->mcEventWeight(lhe_idx) : 0.; 
        }       
    }
    if (m_writemask & WriteOpts::writePRW) {
        m_prwWeight = acc_PuW(*evt_info);
        m_rnd_run = acc_Random(*evt_info);
        m_rnd_lumi_block = acc_LumiBlock(*evt_info);
    }
    if (m_writemask & WriteOpts::writeBeamSpot) {
        m_beamSpotWeight = evt_info->beamSpotWeight();
    }
    return true;
}
bool EventInfoBranch::init() { 
    return declare_dependency(m_key) && std::find_if(m_prwKeys.begin(), m_prwKeys.end(), 
                                                     [this](EvtInfoDecor& decor){
                                                        return !declare_dependency(decor);
                                                     }) == m_prwKeys.end();
}
}
