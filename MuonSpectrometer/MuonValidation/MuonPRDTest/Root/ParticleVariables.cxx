
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonPRDTest/ParticleVariables.h>
#include <StoreGate/ReadHandle.h>

namespace MuonPRDTest {
    ParticleVariables::ParticleVariables(MuonTesterTree& tree, 
                                         const std::string& containerKey,
                                         const std::string& outName,
                                         MSG::Level msglvl) :
        PrdTesterModule(tree, "Particles"+ containerKey+outName, msglvl), 
        m_key{containerKey},
        m_branch{std::make_shared<IParticleFourMomBranch>(tree, outName)} {}

    bool ParticleVariables::fill(const EventContext& ctx) {
        SG::ReadHandle readHandle{m_key, ctx};
        if(!readHandle.isPresent()) {
            ATH_MSG_FATAL("Failed to retrieve "<<m_key.fullKey());
            return false;
        }       
        for (const xAOD::IParticle* particle : *readHandle){
            m_branch->push_back(particle);
        }
        return true;
    }
    bool ParticleVariables::declare_keys() {
        return declare_dependency(m_key) && 
               std::ranges::find_if(m_decorKeys,[this](DecorKey_t& key) {
                    return !declare_dependency(key);
               }) == m_decorKeys.end() && 
                parent().addBranch(m_branch);
    } 
    void ParticleVariables::declare_decorator(const std::string& decorName) {
        if (decorName.empty()){
            ATH_MSG_WARNING("Empty decorator names are not allowed");
        } else {
            m_decorKeys.emplace_back(m_key, decorName);
            ATH_MSG_DEBUG("Declare new dependency on "<<m_decorKeys.back().fullKey());
        }
    }
    bool ParticleVariables::addVariable(std::shared_ptr<IParticleDecorationBranch> branch) {
        return m_branch->addVariable(std::move(branch));
    }
    size_t ParticleVariables::push_back(const xAOD::IParticle* p) {
        m_branch->push_back(p);
        return m_branch->find(p);
    }

}
