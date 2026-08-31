/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonPRDTestR4/TrackContainerModule.h"

#include "StoreGate/ReadHandle.h"
#include "ActsInterop/UnitConverters.h"
#include "ActsEvent/Decoration.h"

#include "xAODMuon/Muon.h"

#include "Acts/Propagator/detail/PointwiseMaterialInteraction.hpp"


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
        constexpr auto iterColumn = Acts::hashString("Gx2fnUpdateColumn");
        if (!actsTrk || !actsTrk->hasColumn(iterColumn)) {
            VectorBranch<std::uint16_t>::push_back(0);
            return;
        }
        VectorBranch<std::uint16_t>::push_back(actsTrk->component<std::uint32_t>(iterColumn)); 
    }
    void TrackFitIterBranch::push_back(const xAOD::IParticle& p) { push_back(&p); }
    void TrackFitIterBranch::operator+=(const xAOD::IParticle* p) { push_back(p); }
    void TrackFitIterBranch::operator+=(const xAOD::IParticle& p) { push_back(p); }


    MaterialRecorderBranch::MaterialRecorderBranch(MuonVal::IParticleFourMomBranch& parent):
        MuonVal::VectorBranch<float>{parent.tree(), std::format("{:}_materialL0", parent.name())},
        m_thickX0{std::make_unique<MuonVal::VectorBranch<float>>(parent.getTree(),
                                        std::format("{:}_materialX0", parent.name()))},
        m_nStates{std::make_unique<MuonVal::VectorBranch<std::uint8_t>>(parent.getTree(),
                                        std::format("{:}_nMaterialStates", parent.name()))} {
        parent.getTree().addBranch(m_thickX0);
        parent.getTree().addBranch(m_nStates);
        m_thickX0 = parent.getTree().getBranch<MuonVal::VectorBranch<float>>(m_thickX0->name());
        m_nStates = parent.getTree().getBranch<MuonVal::VectorBranch<std::uint8_t>>(m_nStates->name());
    }
    void MaterialRecorderBranch::push_back(const xAOD::IParticle& p) {push_back(&p); }
    void MaterialRecorderBranch::operator+=(const xAOD::IParticle* p) {push_back(p); }
    void MaterialRecorderBranch::operator+=(const xAOD::IParticle& p) {push_back(p); }
    void MaterialRecorderBranch::push_back(const xAOD::IParticle* p) {
       
        const xAOD::TrackParticle* trk = nullptr;
        if (p->type() == xAOD::Type::ObjectType::TrackParticle) {
            trk = static_cast<const xAOD::TrackParticle*>(p);    
        } else if (p->type() == xAOD::Type::ObjectType::Muon) {
            trk = static_cast<const xAOD::Muon*>(p)->trackParticle(xAOD::Muon::TrackParticleType::Primary);
        } else {
            THROW_EXCEPTION("No track particle object has been given to " <<name());
        }
        
        float L0{0.f}, X0{0.f};
        std::uint8_t nStates{0};
        auto actsTrk = ActsTrk::getActsTrack(*trk);
        const ActsTrk::GeometryContext* geoCtx{nullptr};
        SG::get(geoCtx, m_geoCtxKey, Gaudi::Hive::currentContext()).ignore();
        for (auto state : actsTrk->trackStates()) {
            if (!state.typeFlags().hasMaterial()) {
                continue;
            }

            Acts::BoundTrackParameters trkPars = actsTrk->createParametersFromState(state);

            auto res = Acts::detail::evaluateMaterialSlab(geoCtx->context(),
                                                          trkPars.referenceSurface(),
                                                          Acts::Direction::Forward(),
                                                          trkPars.position(geoCtx->context()),
                                                          trkPars.direction(),
                                                          Acts::MaterialUpdateMode::FullUpdate);
            
            if (!res.ok()) {
                continue;
            }
            const Acts::MaterialSlab& slab{*res};
            
            if (slab.isVacuum()) {
                ATH_MSG_DEBUG("Encountered vacuum at "<<trkPars);
                continue;
            }
            ++nStates;
            L0 += slab.thicknessInL0();
            X0 += slab.thicknessInX0();
        }
        m_nStates->push_back(nStates);
        m_thickX0->push_back(X0);
        MuonVal::VectorBranch<float>::push_back(L0);
    }
    bool MaterialRecorderBranch::init() {
        return MuonVal::VectorBranch<float>::init() && declare_dependency(m_geoCtxKey);
    }
    

    EnergyLossBranch::EnergyLossBranch(MuonVal::IParticleFourMomBranch& parent):
        MuonVal::VectorBranch<float>{parent.tree(), std::format("{:}_eLoss",parent.name())}{} 

    void EnergyLossBranch::push_back(const xAOD::IParticle* p) { 
         const xAOD::TrackParticle* trk = nullptr;
        if (p->type() == xAOD::Type::ObjectType::TrackParticle) {
            trk = static_cast<const xAOD::TrackParticle*>(p);    
        } else if (p->type() == xAOD::Type::ObjectType::Muon) {
            trk = static_cast<const xAOD::Muon*>(p)->trackParticle(xAOD::Muon::TrackParticleType::Primary);
        } else {
            THROW_EXCEPTION("No track particle object has been given to " <<name());
        }
        auto actsTrk = ActsTrk::getActsTrack(*trk);
        const Acts::BoundTrackParameters perigee = actsTrk->createParametersAtReference();
        for (const auto state :  actsTrk->trackStatesReversed()) {
            const Acts::BoundTrackParameters statePars = actsTrk->createParametersFromState(state);
            push_back(statePars.absoluteMomentum() - perigee.absoluteMomentum());
            return;
        } 
    }
    void EnergyLossBranch::push_back(const xAOD::IParticle& p) { push_back(&p); }
    void EnergyLossBranch::operator+=(const xAOD::IParticle* p)  {push_back(p); }
    void EnergyLossBranch::operator+=(const xAOD::IParticle& p) { push_back(p); }
}
