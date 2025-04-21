
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonTesterTree/IParticleFourMomBranch.h>
#include <xAODTracking/TrackParticle.h>
#include <xAODTruth/TruthParticle.h>
namespace {
    constexpr size_t dummyIdx = -1;
    constexpr float MeVtoGeV = 1.e-3;
    static const SG::AuxElement::ConstAccessor<float> acc_charge{"charge"};
}


namespace MuonVal{
    IParticleFourMomBranch::IParticleFourMomBranch(MuonTesterTree& tree,
                                               const std::string& particle): 
        AthMessaging{"IParticle "+particle},
        m_parent{tree},
        m_name{particle}{}
    
    MuonTesterTree& IParticleFourMomBranch::getTree() {
        return m_parent;
    }
    const std::vector<const xAOD::IParticle*>& IParticleFourMomBranch::getCached()const {
        return m_cached_particles;
    }

    size_t IParticleFourMomBranch::size() const { return m_updated ? m_cached_particles.size() : 0; }
    std::string IParticleFourMomBranch::name() const { return m_name; }
    const TTree* IParticleFourMomBranch::tree() const { return m_pt.tree(); }
    TTree* IParticleFourMomBranch::tree() { return m_pt.tree(); }
    void IParticleFourMomBranch::operator+=(const xAOD::IParticle* p) {push_back(p); }
    void IParticleFourMomBranch::operator+=(const xAOD::IParticle& p) {push_back(p); }
    void IParticleFourMomBranch::push_back(const xAOD::IParticle& p) { push_back(&p); }
    void IParticleFourMomBranch::push_back(const xAOD::IParticle* p) {
        if (!m_updated) {
            m_cached_particles.clear();
            m_updated = true;
        }
        /// Avoid that the particle is added twice to the tree
        if (!p || find(p) < size()){
            if (p) ATH_MSG_VERBOSE("Rejected particle ("<<p<<") "<<p->pt()<<", "<<p->eta()<<", "<<p->phi()<<". Size: "<<size());
            return;
        }

        m_cached_particles.push_back(p);
        m_pt.push_back(p->pt() * MeVtoGeV);
        m_eta.push_back(p->eta());
        m_phi.push_back(p->phi());
        m_e.push_back(p->e() * MeVtoGeV);
        int q{0};
        if (acc_charge.isAvailable(*p)) {
            q = acc_charge(*p);
        } else if (p->type() == xAOD::Type::ObjectType::TrackParticle){
            q = static_cast<const xAOD::TrackParticle*>(p)->charge();
        } else if (p->type() == xAOD::Type::ObjectType::TruthParticle) {
            q = static_cast<const xAOD::TruthParticle*>(p)->charge();
        }
        ATH_MSG_VERBOSE("New particle ("<<p<<") "<<p->pt()<<", "<<p->eta()<<", "<<p->phi()<<", q: "<<q<<". Size: "<<size());

        m_q.push_back(q);
        for (const auto& var : m_variables) {
            var->push_back(p);
        }
    }

    size_t IParticleFourMomBranch::find(const xAOD::IParticle& p) const {
        return find(&p);
    }
    size_t IParticleFourMomBranch::find(const xAOD::IParticle* p) const {
        if (!p) {
            return dummyIdx;
        }
        return find([p](const xAOD::IParticle* cached) { 
            return p == cached; 
        });
    }
    size_t IParticleFourMomBranch::find(std::function<bool(const xAOD::IParticle*)> func) const {
        if (!m_updated) {
            return dummyIdx;
        }
        size_t j{0};
        for (const xAOD::IParticle* p : m_cached_particles) {
            if (func(p)) {
                return j;
            }
            ++j;
        }
        return dummyIdx;
    }
    bool IParticleFourMomBranch::fill(const EventContext& ctx) {
        if (!m_updated) {
            m_cached_particles.clear();
        }
        if (!m_pt.fill(ctx) || !m_eta.fill(ctx) || !m_phi.fill(ctx) || !m_e.fill(ctx) || !m_q.fill(ctx)){
            return false;
        }
        for (const auto& var : m_variables) {
            if (!var->fill(ctx)) {
                return false;
            }
        }
        m_init = true;
        m_updated = false;
        return true;
    }
    bool IParticleFourMomBranch::initialized() const {
        return m_init;
    }
    bool IParticleFourMomBranch::init() {
        if (initialized()) {
            return true;
        }

        if (!m_pt.init() || !m_eta.init() || !m_phi.init() || !m_e.init() || !m_q.init()) {
            return false;
        }
        std::vector<IMuonTesterBranch*> branchesToInit{};
        for (auto& v : m_variables) {
            branchesToInit.push_back(v.get());
        }
        std::sort(branchesToInit.begin(), branchesToInit.end(),
                [](IMuonTesterBranch* a, IMuonTesterBranch* b) {
                    return a->name() < b->name();
                });
        for (auto& br : branchesToInit) {
            if (!br->init()) {
                ATH_MSG_ERROR("Failed to initialize "<<br->name());
                return false;
            }
        }
        return true;
    }

    bool IParticleFourMomBranch::addVariable(std::shared_ptr<IParticleDecorationBranch> branch) {
        if (initialized()) {
            ATH_MSG_ERROR("The branch is already initialized. Cannot add more branches");
            return false;
        } else if (!branch) {
           ATH_MSG_ERROR("Nullptr is given.");
            return false;
        }

        auto itr = std::find_if(m_variables.begin(), m_variables.end(),
                                [&branch](const std::shared_ptr<IParticleDecorationBranch>& known) {
                                    return known == branch || known->name() == branch->name();
                                });
        if (itr != m_variables.end()) {
            if (typeid((*itr).get()) != typeid(branch.get())) {
                ATH_MSG_ERROR("Different branches have been added here under "<<branch->name()<<".");
                return false;
            }
            return true;
        }
        m_variables.push_back(branch);
        return m_parent.registerBranch(branch);
    }
    std::vector<IParticleFourMomBranch::DataDependency> IParticleFourMomBranch::data_dependencies() {
        std::vector<DataDependency> to_ret{};
        for (const std::shared_ptr<IParticleDecorationBranch>& br : m_variables) {
            std::vector<DataDependency> childDep{br->data_dependencies()};
            to_ret.insert(to_ret.end(),childDep.begin(), childDep.end());
        }
        return to_ret;
    }
}