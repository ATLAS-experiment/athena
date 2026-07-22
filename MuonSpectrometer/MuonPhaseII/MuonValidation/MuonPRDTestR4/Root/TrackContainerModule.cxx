/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonPRDTestR4/TrackContainerModule.h"

#include "StoreGate/ReadHandle.h"
#include "ActsInterop/UnitConverters.h"
#include "ActsEvent/Decoration.h"

#include "xAODMuon/Muon.h"
using namespace Acts;
namespace MuonValR4{


    TrackFitIterBranch::TrackFitIterBranch(IParticleFourMomBranch& parent):
        MuonVal::VectorBranch<std::uint16_t>{parent.tree(), std::format("{:}_nIter",parent.name())}{}
    void TrackFitIterBranch::push_back(const xAOD::IParticle* p) {
        const xAOD::TrackParticle* trk = nullptr;
        if (p->type() == xAOD::Type::ObjectType::TrackParticle) {
            trk = static_cast<const xAOD::TrackParticle*>(p);    
        } else if (p->type() == xAOD::Type::ObjectType::Muon) {
            trk = static_cast<const xAOD::Muon*>(p)->trackParticle(xAOD::Muon::TrackParticleType::Primary);
        } else {
            THROW_EXCEPTION("No track particle object has been given to " <<name());
        }

        auto actsTrk = ActsTrk::getActsTrack(*trk);
        constexpr auto iterColumn = Acts:: hashString("Gx2fnUpdateColumn");
        if (!actsTrk || !actsTrk->hasColumn(iterColumn)) {
            VectorBranch<std::uint16_t>::push_back(0);
            return;
        }
        VectorBranch<std::uint16_t>::push_back(actsTrk->component<std::uint32_t>(iterColumn)); 
    }
    void TrackFitIterBranch::push_back(const xAOD::IParticle& p) { push_back(&p); }
    void TrackFitIterBranch::operator+=(const xAOD::IParticle* p) { push_back(p); }
    void TrackFitIterBranch::operator+=(const xAOD::IParticle& p) { push_back(p); }

}
