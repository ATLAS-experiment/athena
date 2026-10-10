/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TrackingGeometryTest.h"

#include "GeoPrimitives/GeoPrimitivesHelpers.h"

#include "ActsInterop/LoggerUtils.h"
#include "ActsInterop/UnitConverters.h" 

#include "ActsGeometryInterfaces/ISurfacePlacement.h"
#include "xAODMuon/MuonSegmentContainer.h"

#include "xAODTruth/TruthVertex.h"
#include "MuonTruthHelpers/MuonSimHitHelpers.h"
#include "TruthUtils/HepMCHelpers.h"

#include "MuonTesterTree/EventInfoBranch.h"

//ACTS
#include "Acts/Definitions/Units.hpp"
#include "Acts/EventData/ParticleHypothesis.hpp"
#include "Acts/Propagator/ActorList.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Propagator/MaterialInteractor.hpp"
#include "Acts/Propagator/EigenStepper.hpp"
#include "Acts/Propagator/Navigator.hpp"
#include "Acts/Propagator/detail/SteppingLogger.hpp"
#include "Acts/Surfaces/StrawSurface.hpp"
#include "Acts/Utilities/AngleHelpers.hpp"
#include "Acts/Surfaces/detail/PlanarHelper.hpp"

#include <chrono>

namespace {

struct PropagatorRecorder{
    PropagatorRecorder(Acts::BoundTrackParameters&& pars,
                       const Identifier& id):
        extpPars{std::move(pars)},
        surfaceId{id}{}

    Acts::BoundTrackParameters extpPars;
    /// @brief Identifier of the propagated hit
    Identifier surfaceId{}; 
    /** @brief Flag to toggle whether the parameters were matched */
    std::optional<unsigned> matched{};
};

std::optional<Acts::ParticleHypothesis> makeHypothesis(const unsigned pdgId) {
    switch (pdgId) {
        case MC::MUON:
            return Acts::ParticleHypothesis::muon();
        case MC::GEANTINOPLUS:
            return Acts::ParticleHypothesis::chargedGeantino();
        case MC::GEANTINO0:
            return Acts::ParticleHypothesis::geantino();
        case MC::ELECTRON:
            return Acts::ParticleHypothesis::electron();
        default:
            break;
    }
    return std::nullopt;
}

}


namespace MuonValR4 {
    StatusCode TrackingGeometryTest::initialize() {
        ATH_CHECK(AthHistogramAlgorithm::initialize());
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_trackingGeometrySvc.retrieve());
        ATH_CHECK(m_extrapolationTool.retrieve());
        ATH_CHECK(m_truthParticleKey.initialize());
        ATH_CHECK(m_truthSegLinkKey.initialize());
        ATH_CHECK(detStore()->retrieve(m_detMgr));
        ATH_CHECK(m_ctxProvider.initialize());
        int evtOpts{};
        m_tree.addBranch(std::make_unique<MuonVal::EventInfoBranch>(m_tree, evtOpts));
        ATH_CHECK(m_tree.init(this));       
        return StatusCode::SUCCESS;
    }

    StatusCode TrackingGeometryTest::finalize() {
        ATH_CHECK(m_tree.write());
        return StatusCode::SUCCESS;
    }

    std::shared_ptr<const Acts::Surface> 
        TrackingGeometryTest::surface(const Identifier& id) const {
        return m_detMgr->getReadoutElement(id)->surfacePtr(surfaceHash(id));
    }
        /** @brief Returns the hash to fetch a surface */
    IdentifierHash TrackingGeometryTest::surfaceHash(const Identifier& id) const {
        const MuonGMR4::MuonReadoutElement* reElement = m_detMgr->getReadoutElement(id);
        return reElement->detectorType() == ActsTrk::DetectorType::Mdt ?
               reElement->measurementHash(id) : reElement->layerHash(id);
    }
    std::optional<Acts::BoundTrackParameters>
        TrackingGeometryTest::createBoundPars(const Acts::GeometryContext& tgContext,
                                                     const xAOD::MuonSimHit& simHit) const {
        auto hypothesis = makeHypothesis(std::abs(simHit.pdgId()));
        if (!hypothesis) {
            ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - Particle hypothesis making failed. PdgId: "
                <<std::abs(simHit.pdgId()));
            return std::nullopt;
        }
        std::shared_ptr<const Acts::Surface> startSurf = surface(simHit.identify());
        const Acts::Transform3& trf = startSurf->localToGlobalTransform(tgContext);
        Amg::Vector3D locPos = xAOD::toEigen(simHit.localPosition());
        const Amg::Vector3D locDir = xAOD::toEigen(simHit.localDirection());
        if (startSurf->type() == Acts::Surface::SurfaceType::Plane &&
            std::abs(locPos.z()) > Acts::s_onSurfaceTolerance) {
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Hit "<<Amg::toString(locPos)
                        <<" is not exactly expressed on the plane of "
                        <<m_idHelperSvc->toStringGasGap(simHit.identify()));
            using namespace Acts::PlanarHelper;
            auto isect = intersectPlane(locPos, locDir, Amg::Vector3D::UnitZ(), 0.);
            locPos = isect.position();
        }

        const Amg::Vector3D globPos{trf * locPos};
        const Amg::Vector3D globDir = trf.linear() * locDir;
        
        auto pars = Acts::BoundTrackParameters::create(tgContext,
                    startSurf, ActsTrk::convertPosToActs(globPos), globDir, 
                    MC::charge(&simHit) / ActsTrk::energyToActs(simHit.kineticEnergy()), 
                    Acts::BoundMatrix::Identity(), *hypothesis);
        if (!pars.ok()) {
            ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - Failed to create valid parameters from sim hit "
                <<m_idHelperSvc->toString(simHit.identify())<<" @"<<Amg::toString(locPos));
            return std::nullopt;
        }
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" Created track parameters from sim hit "
            <<m_idHelperSvc->toString(simHit.identify())<<"\n"<<(*pars));
        return (*pars);
    }


    StatusCode TrackingGeometryTest::execute(const EventContext& ctx) {
        const xAOD::TruthParticleContainer* truthParticles{nullptr};
        ATH_CHECK(SG::get(truthParticles, m_truthParticleKey, ctx));

        const Acts::GeometryContext tgContext = m_ctxProvider.getGeometryContext(ctx);

        auto trkGeo = m_trackingGeometrySvc->trackingGeometry();
        ActsTrk::IExtrapolationTool::SurfaceRecordOptions propOptions{trkGeo->highestTrackingVolume()};
        propOptions.recordSensitive = true;
        propOptions.recordMaterial = false;
        propOptions.recordPassive = false;

        auto fillPropResult = [&](const PropagatorRecorder& record) {
            m_propLocPos.push_back(record.extpPars.localPosition());
            m_propDir.push_back(record.extpPars.direction());
            m_propGlobPos.push_back(record.extpPars.position(tgContext));
            m_propP.push_back(record.extpPars.absoluteMomentum());
            m_propQ.push_back(record.extpPars.charge());
            m_propSurfType.push_back(Acts::toUnderlying(record.extpPars.referenceSurface().type()));
            m_propHitLink.push_back(record.matched.value_or(-1));
            m_propGeoId.push_back(record.extpPars.referenceSurface().geometryId().value());
        };

        for(const xAOD::TruthParticle* truthParticle : *truthParticles) {
           //select only muons or geantinos particles
            auto actsParticleHypothesis = makeHypothesis(truthParticle->absPdgId());
            if (!actsParticleHypothesis) {
                continue;
            }
            ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Processing truth particle with PDG ID: " << truthParticle->absPdgId() << " ,pT: "
                        << truthParticle->pt() << " , p: " << truthParticle->p4().P() << ", eta: " << truthParticle->eta() << " , phi: " << truthParticle->phi());

            std::vector<std::pair<const xAOD::MuonSegment*, std::vector<const xAOD::MuonSimHit*>>> muonSegmentWithSimHits;
            for(const xAOD::MuonSegment* seg: MuonR4::getTruthSegments(*truthParticle)) {
                auto unordedHits = MuonR4::getMatchingSimHits(*seg);
                std::vector<const xAOD::MuonSimHit*> muonSimHits{unordedHits.begin(), unordedHits.end()};           
                ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Segment at : "<<Amg::toString(seg->position())<<" with Sim Hits: "<<unordedHits.size());
                muonSegmentWithSimHits.emplace_back(seg,std::move(muonSimHits));
            }
            if (muonSegmentWithSimHits.empty()) {
                ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - No segments associated");
                continue;
            }

            const auto& particle = truthParticle->p4();

            m_truthPt = particle.Pt();
            m_eta = particle.Eta();
            m_phi = particle.Phi();
            m_q = truthParticle->charge();
            m_pdgId = truthParticle->absPdgId();
        
            const xAOD::TruthVertex* prodVertex = truthParticle->prodVtx();
            const Amg::Vector3D prodPos = prodVertex ? Amg::Vector3D(prodVertex->x(), prodVertex->y(), prodVertex->z()) 
                                                     : Amg::Vector3D::Zero();
            const Amg::Vector3D prodDir{Amg::dirFromAngles(particle.Phi(), particle.Theta())};
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Truth particle produced at " << Amg::toString(prodPos) 
                        << " with direction " << Amg::toString(prodDir));

            //position, direction and momentum at the start of the propagation
   
            Acts::BoundTrackParameters start = Acts::BoundTrackParameters::createCurvilinear(
                    ActsTrk::convertPosToActs(prodPos), prodDir,
                    truthParticle->charge() / ActsTrk::energyToActs(particle.P()),
                    Acts::BoundMatrix::Identity(), *actsParticleHypothesis);

            if(m_startFromFirstHit) {
                // sort the segments by their radial distance
                std::ranges::sort(muonSegmentWithSimHits, 
                    [&](const auto& a, const auto& b) {
                        return (a.first->position() - prodPos).dot(prodDir) < 
                               (b.first->position() - prodPos).dot(prodDir);
                    });
            
                //sort the sim hits of the first segment to get the starting propagation position as the first one
                std::vector<const xAOD::MuonSimHit*>& muonSimHits = muonSegmentWithSimHits.front().second;
                std::ranges::sort(muonSimHits, 
                    [&](const xAOD::MuonSimHit* hit1, const xAOD::MuonSimHit* hit2){
                    const Amg::Vector3D globalPos1 = surface(hit1->identify())->localToGlobalTransform(tgContext)
                                                    *xAOD::toEigen(hit1->localPosition());
                    const Amg::Vector3D globalPos2 = surface(hit2->identify())->localToGlobalTransform(tgContext)
                                                    *xAOD::toEigen(hit2->localPosition());
                    return globalPos1.norm()<globalPos2.norm();
                });
             
                ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - After sorting..");
                for(const auto& simHit: muonSimHits){
                    const Acts::Transform3& trf = surface(simHit->identify())->localToGlobalTransform(tgContext);
                    const Amg::Vector3D globPos = trf*xAOD::toEigen(simHit->localPosition());
                    ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Sim Hit of first segment at position: "<<Amg::toString(globPos));
                
                }
                auto firstPars = createBoundPars(tgContext, *muonSimHits.front());
                if (!firstPars) {
                    ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - Start parameter creation failed");
                    continue;
                }
                start = std::move(*firstPars);
            }

            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Starting propagation from \n"<<start);   	

            //measure the propagation time and save it to the ntuple for performance validation
            //start the clock
            const auto propagationStart = std::chrono::steady_clock::now();
            auto propResult = m_extrapolationTool->propagateAndRecord(ctx, start, propOptions);
            const auto propagationEnd = std::chrono::steady_clock::now(); //stop the clock
            if (!propResult.ok()) {
                ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - Propagation failed");
                continue;
            }
            m_propTime = (std::chrono::duration<double>(propagationEnd - propagationStart).count()) * 1000;
           
            ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - [clock time]: "<< m_propTime<<" ms");
            std::vector<PropagatorRecorder> propagatedHits;

            for(Acts::BoundTrackParameters& trackPars : (*propResult)) {
                
                const auto* sCache = dynamic_cast<const ActsTrk::ISurfacePlacement*>(trackPars.referenceSurface().surfacePlacement());
                if(!sCache) {
                    ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Surface found but it's a portal, continuing..");
                    continue;
                }
                const Identifier ID = sCache->identify();
                ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Identify propagated hit " << m_idHelperSvc->toString(ID) 
                        << " with local parameters \n" << trackPars);
                
                propagatedHits.emplace_back(std::move(trackPars), ID);
            }
            /** Loop first over the sim hits and match them with the propagated records */
            for (const auto&[_, simHits] : muonSegmentWithSimHits) {
                for(const xAOD::MuonSimHit* simHit : simHits){
                    auto boundHitPars = createBoundPars(tgContext, *simHit);
                    if (!boundHitPars) {
                        continue;
                    }
                    const Identifier id = simHit->identify();
                    m_detId.push_back(id);
                    m_techIdx.push_back(Acts::toUnderlying(m_idHelperSvc->technologyIndex(id)));
                    m_gasGapId.push_back(surfaceHash(id));
                    /// Store the truth parameters
                    m_truthLoc.push_back(boundHitPars->localPosition());
                    m_truthDir.push_back(boundHitPars->direction());
                    m_truthGlob.push_back(boundHitPars->position(tgContext));
                    m_truthP.push_back(boundHitPars->absoluteMomentum());
                    m_truthGeoId.push_back(boundHitPars->referenceSurface().geometryId().value());
           
                    /** Get the matching propagated hits */
                    auto it_begin = std::ranges::find_if(propagatedHits,
                                        [&](const PropagatorRecorder& propagatedHit) {
                                        return propagatedHit.extpPars.referenceSurface().geometryId() ==
                                               boundHitPars->referenceSurface().geometryId();
                    });
                    if (it_begin == propagatedHits.end()) {
                        m_isPropagated.push_back(-1);
                        continue;
                    }
                    //find - if any - propagated hits on the same layer 
                    auto it_end = std::find_if(it_begin, propagatedHits.end(),
                        [&](const PropagatorRecorder& propagatedHit) {
                            return propagatedHit.extpPars.referenceSurface().geometryId() !=
                                    boundHitPars->referenceSurface().geometryId();
                        });
                    /// Mutliple propagations may match which is a bit odd 
                    /// but find the closest parameter set
                    if (std::distance(it_begin, it_end) > 1) {
                        ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Found multiple intersections to the same surface "
                                <<boundHitPars->referenceSurface().geometryId()
                                <<", take the closest one");
                         /// find the closest propagated hit to the truth hit
                         auto it = std::min_element(it_begin, it_end,
                                             [&](const PropagatorRecorder& a,
                                                 const PropagatorRecorder& b){
                             return (a.extpPars.localPosition() - boundHitPars->localPosition()).mag2() < 
                                    (b.extpPars.localPosition() - boundHitPars->localPosition()).mag();
                        }); 
                        if (it != it_begin) {
                            ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Updated the matched track parameters from\n "
                                <<it_begin->extpPars<<"\nto\n"
                                <<it->extpPars);
                        }
                        it_begin = it;
                    }
                    m_isPropagated.push_back(m_propHitLink.size());
                    it_begin->matched = m_isPropagated.size() -1;
                    
                    fillPropResult(*it_begin);
                }
            }
            auto [begin, end] = std::ranges::remove_if(propagatedHits, 
                                                       [](const PropagatorRecorder& result){
                                                            return result.matched.has_value();
                                                       });
            propagatedHits.erase(begin, end);
            ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Fill "<<propagatedHits.size()<<" unmatched propagations");
            for (const PropagatorRecorder& result : propagatedHits) {
                fillPropResult(result);
            }
            m_tree.fill(ctx); 
        }


        return StatusCode::SUCCESS;
    }

}

