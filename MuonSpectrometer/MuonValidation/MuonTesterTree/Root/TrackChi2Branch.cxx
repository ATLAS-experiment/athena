/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonTesterTree/TrackChi2Branch.h>
#include <MuonTesterTree/throwExcept.h>
#include <xAODMuon/Muon.h>
namespace MuonVal{
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
}
