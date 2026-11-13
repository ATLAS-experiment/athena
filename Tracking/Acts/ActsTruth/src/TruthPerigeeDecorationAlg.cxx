

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "TruthPerigeeDecorationAlg.h"

#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"
#include "xAODMuonViews/ContainerDecorator.h"
#include "xAODTruth/TruthVertex.h"
#include "TruthUtils/HepMCHelpers.h"
#include "Acts/Surfaces/PerigeeSurface.hpp"


namespace ActsTrk{
    StatusCode TruthPerigeeDecorationAlg::initialize() {
        ATH_CHECK(m_extrapolationTool.retrieve());
        ATH_CHECK(m_readKey.initialize());
        ATH_CHECK(m_d0Key.initialize());
        ATH_CHECK(m_z0Key.initialize());
        ATH_CHECK(m_ZProdKey.initialize());
        ATH_CHECK(m_RProdKey.initialize());
        ATH_CHECK(m_isDecorKey.initialize());
        return StatusCode::SUCCESS;
    }
    StatusCode TruthPerigeeDecorationAlg::execute(const EventContext& ctx) const {
        const xAOD::TruthParticleContainer* truthParticles{};
        ATH_CHECK(SG::get(truthParticles, m_readKey, ctx));

        xAOD::ContainerDecorator dec_d0{m_d0Key, ctx, std::numeric_limits<float>::max()};
        xAOD::ContainerDecorator dec_z0{m_z0Key, ctx, std::numeric_limits<float>::max()};
        xAOD::ContainerDecorator dec_zProd{m_ZProdKey, ctx, std::numeric_limits<float>::max()};
        xAOD::ContainerDecorator dec_rProd{m_RProdKey, ctx, std::numeric_limits<float>::max()};
        xAOD::ContainerDecorator dec_isDecorated{m_isDecorKey, ctx, std::uint8_t{0}};
        auto perigeeSurf = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3::Zero());
        for (const xAOD::TruthParticle* truthPart : *truthParticles) {
            if (!truthPart->isCharged() || !MC::isStable(truthPart) ||
                HepMC::simulation_history(truthPart, -1).size() > 1 ) {
                continue;
            }
            const xAOD::TruthVertex* prodVtx = truthPart->prodVtx();
            if (!prodVtx) {
                continue;
            }
            const Amg::Vector3D prodPos{prodVtx->x(), prodVtx->y(), prodVtx->z()};

            const Acts::BoundTrackParameters start = Acts::BoundTrackParameters::createCurvilinear(
                ActsTrk::convertPosToActs(prodPos), 
                Acts::makeDirectionFromPhiEta(truthPart->phi(), truthPart->eta()),
                truthPart->charge() / ActsTrk::energyToActs(truthPart->pt() * std::cosh(truthPart->eta())), std::nullopt, Acts::ParticleHypothesis::muon());
            /// The point of closest approach lies behind the production vertex if the transverse momentum points away from the beam line
            const Acts::Direction navDir = prodPos.block<2,1>(0,0).dot(start.direction().block<2,1>(0,0))  > 0. 
                                         ? Acts::Direction::Backward() : Acts::Direction::Forward();
            auto perigee = m_extrapolationTool->propagate(ctx, start, *perigeeSurf, navDir);
            if (!perigee.ok()) {
                ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Propagation of the truth muon from "<<Amg::toString(prodPos)
                            <<" to the beam line failed: "<<perigee.error().message());
                continue;
            }
            dec_d0(*truthPart) = perigee->parameters()[Acts::eBoundLoc0];
            dec_z0(*truthPart) = perigee->parameters()[Acts::eBoundLoc1];
            dec_zProd(*truthPart) = prodPos.z();
            dec_rProd(*truthPart) = prodPos.perp();
            dec_isDecorated(*truthPart) = true;
        }
        return StatusCode::SUCCESS;
    }
}