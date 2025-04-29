/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonTesterTree/LinkerBranch.h>
#include <format>
namespace MuonVal{
    LinkerBranch::LinkerBranch(IParticleFourMomBranch& parent,
                               ParticleBranch_ptr linkColl,
                               Linker_t  linker,
                               const std::string& altName):
        VectorBranch<unsigned short>{parent.tree(), 
                                     std::format("{:}_{:}Link", parent.name(), altName.empty() ? linkColl->name() : altName)},
        m_linkColl{linkColl},
        m_linkerFunc{std::move(linker)} {}

    void LinkerBranch::operator+=(const xAOD::IParticle* p) {
        push_back(p);
    }
    void LinkerBranch::operator+=(const xAOD::IParticle& p) {
        push_back(p);
    }
    void LinkerBranch::push_back(const xAOD::IParticle& p) {
        push_back(&p);
    }
    void LinkerBranch::push_back(const xAOD::IParticle* p) {
        const xAOD::IParticle* related = m_linkerFunc(p);
        ParticleBranch_ptr linkColl = m_linkColl.lock();
        linkColl->push_back(related);
        VectorBranch<unsigned short>::push_back(linkColl->find(related));
    }

    
    bool BilateralLinkerBranch::connectCollections(ParticleBranch_ptr primColl,
                                                   ParticleBranch_ptr secondColl,
                                                   Linker_t fromPrimToSec,
                                                   const std::string& altPrimName,
                                                   const std::string& altSecName) {
        return primColl->addVariable(std::make_unique<LinkerBranch>(*primColl, secondColl, fromPrimToSec, altPrimName)) &&
               secondColl->addVariable(std::unique_ptr<IParticleDecorationBranch>{new BilateralLinkerBranch(*secondColl, primColl, fromPrimToSec, altSecName)});
    }
                     
    BilateralLinkerBranch::BilateralLinkerBranch(IParticleFourMomBranch& parent,
                                                 ParticleBranch_ptr primColl,
                                                 Linker_t linker,
                                                 const std::string& altName):
        VectorBranch<unsigned short>{parent.tree(), 
                                     std::format("{:}_{:}Link", parent.name(), altName.empty() ? primColl->name() : altName)},
        m_parent{parent},
        m_linkColl{primColl},
        m_linkerFunc{std::move(linker)} {
        setDefault(-1);
    }

    void BilateralLinkerBranch::push_back(const xAOD::IParticle* /* p*/) {
    }
    void BilateralLinkerBranch::push_back(const xAOD::IParticle& p) {
        push_back(&p);
    }
    void BilateralLinkerBranch::operator+=(const xAOD::IParticle* p){
        push_back(p);
    }
    void BilateralLinkerBranch::operator+=(const xAOD::IParticle& p){
        push_back(p);
    }          
    bool BilateralLinkerBranch::fill(const EventContext& ctx) {
        if (m_parent.size()) {
            /** Allocate the memory */
            get(m_parent.size() -1);
            ParticleBranch_ptr linkColl = m_linkColl.lock();
            const std::vector<const xAOD::IParticle*>& linkeMe = linkColl->getCached();
            for (std::size_t primToSec = 0 ; primToSec < linkeMe.size(); ++primToSec) {
                const size_t linkIdx = m_parent.find(m_linkerFunc(linkeMe[primToSec]));
                if (linkIdx < size()) {
                    get(linkIdx) = primToSec;
                }
            }
        }
        ATH_MSG_VERBOSE("Fill "<<name()<<", size: "<<m_parent.size()<<"/"<<size()<<", "<<m_parent.name());
        return VectorBranch<unsigned short>::fill(ctx);
    }
}