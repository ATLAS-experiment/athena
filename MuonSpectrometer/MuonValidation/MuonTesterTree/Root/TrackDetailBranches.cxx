/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonTesterTree/TrackDetailBranches.h"
#include "MuonTesterTree/throwExcept.h"

#ifndef XAOD_ANALYSIS
#   include "TrkMaterialOnTrack/ScatteringAngles.h"
#endif
#include "xAODMuon/Muon.h"
namespace MuonVal{

    /**********************************************************
     *                  TrackChi2Branch 
     **********************************************************/
    TrackChi2Branch::TrackChi2Branch(IParticleFourMomBranch& parent):
        VectorBranch<float>(parent.tree(), parent.name() + "_chi2"),
        m_nDoF {std::make_shared<VectorBranch<unsigned int>>(parent.tree(), parent.name() + "_nDoF")}
        {
            parent.getTree().addBranch(m_nDoF);
            m_nDoF = parent.getTree().getBranch<VectorBranch<unsigned int>>(m_nDoF->name());
    }
    void TrackChi2Branch::push_back(const xAOD::IParticle* p) {
        const xAOD::TrackParticle* trk = nullptr;
        if (p->type() == xAOD::Type::ObjectType::TrackParticle) {
            trk = static_cast<const xAOD::TrackParticle*>(p);    
        } else if (p->type() == xAOD::Type::ObjectType::Muon) {
            trk = static_cast<const xAOD::Muon*>(p)->trackParticle(xAOD::Muon::TrackParticleType::Primary);
        } else {
            THROW_EXCEPTION("No track particle object has been given to " <<name());
        }

        push_back(trk->chiSquared());
        m_nDoF->push_back(trk->numberDoF());
    }
    void TrackChi2Branch::push_back(const xAOD::IParticle& p) { push_back(&p); }
    void TrackChi2Branch::operator+=(const xAOD::IParticle* p) { push_back(p); }
    void TrackChi2Branch::operator+=(const xAOD::IParticle& p) { push_back(p); }

#ifndef XAOD_ANALYSIS
    /**********************************************************
     *                  EnergylossBranch 
     **********************************************************/
    EnergylossBranch::EnergylossBranch(IParticleFourMomBranch& parent):
        VectorBranch<float>{parent.tree(), std::format("{:}_eLoss",
                                                       parent.name())} {}
    void EnergylossBranch::push_back(const xAOD::IParticle& p) {push_back(&p);}
    void EnergylossBranch::operator+=(const xAOD::IParticle* p) {push_back(p);}
    void EnergylossBranch::operator+=(const xAOD::IParticle& p) {push_back(p);}

    void EnergylossBranch::push_back(const xAOD::IParticle* p) {
        const xAOD::TrackParticle* trk = nullptr;
        if (p->type() == xAOD::Type::ObjectType::TrackParticle) {
            trk = static_cast<const xAOD::TrackParticle*>(p);    
        } else if (p->type() == xAOD::Type::ObjectType::Muon) {
            trk = static_cast<const xAOD::Muon*>(p)->trackParticle(xAOD::Muon::TrackParticleType::Primary);
        } else {
            THROW_EXCEPTION("No track particle object has been given to " <<name());
        }
        const Trk::Track* track = trk->track();
        if (!track) {
            ATH_MSG_WARNING("Reconstructed track is not available "<<name());
            push_back(0.f);
            return;
        }   
        const Trk::TrackParameters* parsFirst{nullptr}, *parsLast{nullptr};
        for (const Trk::TrackStateOnSurface* tsos : *track->trackStateOnSurfaces()) {
            if (!tsos->measurementOnTrack()) {
                continue;
            }
            if (!parsFirst) {
                parsFirst = tsos->trackParameters();
            }
            parsLast = tsos->trackParameters();
        }
        assert(parsFirst != nullptr);
        assert(parsLast != nullptr);
        /// Convert to GeV
        push_back((parsLast->momentum().mag() - parsFirst->momentum().mag())* 1.e-3);
        
    }

    /**********************************************************
     *                  ScatteringBranch 
     **********************************************************/
    ScatteringBranch::ScatteringBranch(IParticleFourMomBranch& parent):
        Base_t{parent.tree(), std::format("{:}_scatPhi", parent.name())},
        m_deltaTheta{std::make_unique<Base_t>(parent.tree(), std::format("{:}_scatTheta", parent.name()))},
        m_sigmaPhi{std::make_unique<Base_t>(parent.tree(),  std::format("{:}_scatSigmaPhi", parent.name()))},
        m_sigmaTheta{std::make_unique<Base_t>(parent.tree(),std::format("{:}_scatSigmaTheta", parent.name()))},
        m_nScat{std::make_unique<VectorBranch<std::uint8_t>>(parent.tree(), std::format("{:}_nScatters", parent.name()))}{
        parent.getTree().addBranch(m_deltaTheta);
        parent.getTree().addBranch(m_sigmaPhi);
        parent.getTree().addBranch(m_sigmaTheta);
        parent.getTree().addBranch(m_nScat);
        m_deltaTheta = parent.getTree().getBranch<Base_t>(m_deltaTheta->name());
        m_sigmaPhi = parent.getTree().getBranch<Base_t>(m_sigmaPhi->name());
        m_sigmaTheta = parent.getTree().getBranch<Base_t>(m_sigmaTheta->name());
        m_nScat = parent.getTree().getBranch<VectorBranch<std::uint8_t>>(m_nScat->name());
    }
    void ScatteringBranch::push_back(const xAOD::IParticle& p) { push_back(&p); }
    void ScatteringBranch::operator+=(const xAOD::IParticle* p) { push_back(p); }
    void ScatteringBranch::operator+=(const xAOD::IParticle& p) { push_back(p); }

    void ScatteringBranch::push_back(const xAOD::IParticle* p) {
        const xAOD::TrackParticle* trk = nullptr;
        if (p->type() == xAOD::Type::ObjectType::TrackParticle) {
            trk = static_cast<const xAOD::TrackParticle*>(p);    
        } else if (p->type() == xAOD::Type::ObjectType::Muon) {
            trk = static_cast<const xAOD::Muon*>(p)->trackParticle(xAOD::Muon::TrackParticleType::Primary);
        } else {
            THROW_EXCEPTION("No track particle object has been given to " <<name());
        }
        std::vector<float> dPhi{}, dTheta{}, sigPhi{}, sigTheta{};
        if (const Trk::Track* track = trk->track(); track != nullptr) {
            for (const Trk::TrackStateOnSurface* tsos : *track->trackStateOnSurfaces()) {
                const auto* angles = dynamic_cast<const Trk::ScatteringAngles*>(tsos->materialEffectsOnTrack());
                if (!angles) {
                    continue;
                }
                dPhi.push_back(angles->deltaPhi());
                dTheta.push_back(angles->deltaTheta());
                sigPhi.push_back(angles->sigmaDeltaPhi());
                sigTheta.push_back(angles->sigmaDeltaTheta());
            }
        }
        push_back(std::move(dPhi));
        m_deltaTheta->push_back(std::move(dTheta));
        m_sigmaPhi->push_back(std::move(sigPhi));
        m_sigmaTheta->push_back(std::move(sigTheta));
    }
#endif
}