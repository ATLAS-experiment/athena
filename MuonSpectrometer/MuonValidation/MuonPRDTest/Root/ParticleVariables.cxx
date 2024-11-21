
/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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
        m_branch{std::make_shared<IParticleFourMomBranch>(tree, outName)}
        {
        }

    bool ParticleVariables::fill(const EventContext& ctx) {
        SG::ReadHandle<xAOD::IParticleContainer> readHandle{m_key, ctx};
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
        return declare_dependency(m_key) && parent().addBranch(m_branch);
    } 
}
