/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "ObjTrackVisualizationAlg.h"
#include "ActsEvent/Decoration.h"
#include "StoreGate/ReadHandle.h"

#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"
#include "MuonVisualizationHelpersR4/ObjVisualizationHelpers.h"
#include "MuonVisualizationHelpersR4/FileHelpers.h"
#include "GeoModelHelpers/StringUtils.h"
#include "ActsInterop/UnitConverters.h"
#include "ActsEvent/ParticleHypothesisEncoding.h"

#include "Acts/Visualization/ObjVisualization3D.hpp"
#include "Acts/Visualization/GeometryView3D.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/Definitions/Units.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"


#include <format>

using namespace Acts::UnitLiterals;
using namespace MuonValR4;

/** @brief Pipe a number to a string. Replace the decimal point by a p and
 *         the negative sign by a m */
std::string toString(const float number) {
    using namespace GeoStrUtils;
    return replaceExpInString(
           replaceExpInString(std::format("{:.3f}", number), ".", "p"), "-", "m");
}

namespace ActsTrk {

    StatusCode ObjTrackVisualizationAlg::initialize()  {
        ATH_CHECK(m_extrapolationTool.retrieve());
        ATH_CHECK(m_readKey.initialize());
        ATH_CHECK(m_ctxProvider.initialize());
        ATH_CHECK(m_trackingGeoSvc.retrieve());
        return StatusCode::SUCCESS;
    }
    StatusCode ObjTrackVisualizationAlg::execute(const EventContext& ctx) {
        const xAOD::TrackParticleContainer* tracks{};
        ATH_CHECK(SG::get(tracks, m_readKey, ctx));
        const Acts::GeometryContext tgContext = m_ctxProvider.getGeometryContext(ctx);
        
        const std::string outDir = std::format("{:}/Event_{:}/", m_outPath.value(), 
                                               ctx.eventID().event_number());
        ensureDirectory(outDir);
        Acts::ObjVisualization3D visualHelper{};
        std::unordered_set<std::shared_ptr<const Acts::Surface>> passedSurfaces{};

        const auto trackingGeo = m_trackingGeoSvc->trackingGeometry();

        for (const xAOD::TrackParticle* particle : *tracks) {
            passedSurfaces.clear();
            auto track = getActsTrack(*particle);
            
            std::optional<Acts::BoundTrackParameters> startPars{};
            if (track) {
                startPars = track->createParametersAtReference();
                for (const auto& state : track->trackStates()) {
                    Acts::GeometryView3D::drawSurface(visualHelper, state.referenceSurface(),
                                                  tgContext, Acts::Transform3::Identity(),
                                                  Acts::s_viewSensitive);
                    passedSurfaces.insert(state.referenceSurface().getSharedPtr());
                }
            } else {
                auto startSurf = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Transform3::Identity());
                Acts::BoundVector pars{Acts::BoundVector::Zero()};
                pars[Acts::eBoundLoc0] = particle->d0();
                pars[Acts::eBoundLoc1] = particle->z0();
                pars[Acts::eBoundLoc1] = particle->phi();
                pars[Acts::eBoundTheta] = particle->theta();
                pars[Acts::eBoundQOverP] = particle->charge() / energyToActs(particle->pt() * std::cosh(particle->eta()));
                startPars = Acts::BoundTrackParameters{startSurf, pars, std::nullopt,
                                                       ParticleHypothesis::convert(particle->particleHypothesis())};
            }
            auto steps = m_extrapolationTool->propagationSteps(ctx, *startPars, Acts::Direction::Forward(),  50._m);
            if (!steps.ok()) {
                continue;
            }

            drawPropagation(steps->first, visualHelper);
            std::stringstream sstr{};
            unsigned surfCounter{0};
            for (const auto& step : steps->first) {
                if (!step.surface) {
                    continue;
                }
                sstr<<"  --- "<<(++surfCounter)<<" @"<<Amg::toString(step.position)
                    <<", perp: "<<step.position.perp()
                    <<", theta: "<<step.momentum.theta()
                    <<", phi: "<<step.momentum.phi();
                if (const Acts::TrackingVolume* vol = 
                    trackingGeo->findVolume(step.surface->geometryId().withBoundary(0).withSensitive(0)); vol != nullptr) {
                    sstr<<", volume: "<<vol->volumeName();
                }
                sstr<<", surface @"<<Amg::toString(step.surface->center(tgContext))
                    <<", id: "<<step.surface->geometryId()
                    <<", bounds: "<<step.surface->bounds()<<std::endl;
                if (passedSurfaces.insert(step.surface).second) {
                    Acts::GeometryView3D::drawSurface(visualHelper, *step.surface,
                                                        tgContext, Acts::Transform3::Identity(),
                                                        Acts::s_viewPassive);
                }
            }
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Propagated track particle "<<particle->index()
                <<" with "<<(particle->pt()*1.e-3)<<" [GeV], eta: "<<particle->eta()
                <<", phi: "<<particle->phi()<<", q: "<<particle->charge()
                <<"\n"<<sstr.str());
            visualHelper.write(std::format("{:}/Track_{:}_pt_{:}_eta_{:}_phi_{:}.obj", outDir,
                                            particle->index(), toString(particle->pt() / 1.e3),
                                            toString(particle->eta()), toString(particle->phi())));
            visualHelper.clear();
        }
        return StatusCode::SUCCESS;
    }
}