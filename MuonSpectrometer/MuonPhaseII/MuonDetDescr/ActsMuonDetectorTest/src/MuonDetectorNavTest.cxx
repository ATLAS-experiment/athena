/*
Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonDetectorNavTest.h"

#include <StoreGate/ReadCondHandle.h>
#include "ActsGeoUtils/SurfaceCache.h"

#include "Acts/Definitions/Units.hpp"
#include "Acts/Propagator/ActorList.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Propagator/MaterialInteractor.hpp"
#include "Acts/Propagator/EigenStepper.hpp"
#include "Acts/Propagator/AtlasStepper.hpp"
#include "Acts/Utilities/VectorHelpers.hpp"
#include "Acts/Visualization/ObjVisualization3D.hpp"
#include "Acts/Visualization/GeometryView3D.hpp"
#include "Acts/EventData/ParticleHypothesis.hpp"
#include "Acts/Propagator/detail/SteppingLogger.hpp"

#include "xAODMuon/MuonSegmentContainer.h"
#include "xAODTruth/TruthVertex.h"
#include "MuonTruthHelpers/MuonSimHitHelpers.h"
#include "TruthUtils/HepMCHelpers.h"
#include "MuonReadoutGeometry/MuonReadoutElement.h"
#include "TruthUtils/HepMCHelpers.h"
#include "ActsInterop/LoggerUtils.h"
#include <fstream>
#include <format>
#include <cmath>

using namespace Acts::UnitLiterals;
using namespace Muon::MuonStationIndex;

namespace {

using SegLink_t = std::vector<ElementLink<xAOD::MuonSegmentContainer>>;
static const SG::ConstAccessor<SegLink_t> segAcc{"truthSegmentLinks"};
static const Amg::Vector3D dummyPos{100.*Gaudi::Units::m, 100.*Gaudi::Units::m, 100.*Gaudi::Units::m};
struct PropagatorRecorder{
    /// @brief Position obtained by the ACTS propagator
    Amg::Vector3D actsPropPos{dummyPos};
    /// @brief Position obtained by the ACTS propagator in the global frame
    Amg::Vector3D actsGlobalPos{dummyPos};
    /// @brief Direction obtained by the ACTS propgator
    Amg::Vector3D actsPropDir{Amg::Vector3D::Zero()};
    /// @brief Position obtained by the ATLAS extrapolator
    Amg::Vector3D atlasPropPos{dummyPos};
    /// @brief Position obtained by the ATLAS extrapolator in the global frame
    Amg::Vector3D atlasGlobalPos{dummyPos};
    /// @brief Direction obtained by the ATLAS extrapolator
    Amg::Vector3D atlasPropDir{Amg::Vector3D::Zero()};
    /// @Identifier of the state
    Identifier id{};
    /// @brief Momentum obtained by the ATLAS extrapolator
    double atlasPropabsMomentum{0.};
    /// @brief Momentum obtained by the ACTS propagator
    double actsPropabsMomentum{0.};
    /// @brief The step size from the acts propagator
    double actsStepSize{0.};
};
}


namespace ActsTrk {

Amg::Transform3D MuonDetectorNavTest::toGlobalTrf(const ActsGeometryContext& gctx, const Identifier& hitId) const {
    const MuonGMR4::MuonReadoutElement* reElement = m_r4DetMgr->getReadoutElement(hitId);
    const IdentifierHash trfHash = reElement->detectorType() == ActsTrk::DetectorType::Mdt ?
                                    reElement->measurementHash(hitId) : reElement->layerHash(hitId);            
    return reElement->localToGlobalTrans(gctx, trfHash);
}
//get the transformation to the readout element's local frame - first measurement plane
Amg::Transform3D MuonDetectorNavTest::toLocalTrf(const ActsGeometryContext& gctx, const Identifier& hitId) const {
    const MuonGMR4::MuonReadoutElement* reElement = m_r4DetMgr->getReadoutElement(hitId);          
    return reElement->globalToLocalTrans(gctx, reElement->layerHash(hitId));
}


IdentifierHash MuonDetectorNavTest::layerHash(const Identifier& id) const {
    return m_r4DetMgr->getReadoutElement(id)->layerHash(id);
}

StatusCode MuonDetectorNavTest::initialize() {
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_geoCtxKey.initialize());
    ATH_CHECK(m_fieldCacheCondObjInputKey.initialize());
    ATH_CHECK(m_truthParticleKey.initialize());
    ATH_CHECK(m_truthSegLinkKey.initialize());
    ATH_CHECK(m_detVolSvc.retrieve());
    ATH_CHECK(m_extrapolator.retrieve());
    ATH_CHECK(m_detMgrKey.initialize());
    ATH_CHECK(m_truthSegLinkKey.initialize());
    ATH_CHECK(detStore()->retrieve(m_r4DetMgr));
    ATH_CHECK(m_tree.init(this));

    ATH_MSG_VERBOSE("Successfully initialized MuonDetectorNavTest");
    return StatusCode::SUCCESS;
}

StatusCode MuonDetectorNavTest::finalize() {
    ATH_MSG_VERBOSE("Finalizing MuonDetectorNavTest");
    ATH_CHECK(m_tree.write());
    return StatusCode::SUCCESS;
}

StatusCode MuonDetectorNavTest::execute() {
    const EventContext& ctx{Gaudi::Hive::currentContext()};
    ATH_MSG_DEBUG("Execute in event "<<ctx.eventID().event_number());
    const ActsGeometryContext* gctx{nullptr};
    const xAOD::TruthParticleContainer* truthParticles{nullptr};
    const AtlasFieldCacheCondObj* fieldCondObj{nullptr};
    const MuonGM::MuonDetectorManager* detMgr{nullptr};
    
    ATH_CHECK(SG::get(gctx, m_geoCtxKey, ctx));
    ATH_CHECK(SG::get(truthParticles, m_truthParticleKey, ctx));
    ATH_CHECK(SG::get(fieldCondObj, m_fieldCacheCondObjInputKey, ctx));
    ATH_CHECK(SG::get(detMgr, m_detMgrKey, ctx));


    const Acts::GeometryContext geoContext = gctx->context();
    const Acts::MagneticFieldContext mfContext = Acts::MagneticFieldContext(fieldCondObj);


    using Stepper = Acts::EigenStepper<>;
    //using Stepper = Acts::AtlasStepper;
    using Navigator = Acts::Experimental::DetectorNavigator;
    using Propagator = Acts::Propagator<Stepper,Navigator>;
    using ActorList = Acts::ActorList<Acts::detail::SteppingLogger, Acts::MaterialInteractor, Acts::EndOfWorldReached>;
    using PropagatorOptions = Propagator::Options<ActorList>;

    
    PropagatorOptions options(geoContext, mfContext);

    options.pathLimit = m_pathLimit;
    options.stepping.maxStepSize = m_maxStepSize;
    options.stepping.stepTolerance = m_stepTolerance;
    options.maxSteps = m_maxSteps;
    options.maxTargetSkipping = m_maxTargetSkipping;

    //Configure the material collector
    auto& materialInteractor = options.actorList.get<Acts::MaterialInteractor>();
    materialInteractor.energyLoss = false;
    materialInteractor.multipleScattering = false;
    materialInteractor.recordInteractions = false;
    
    const Acts::Experimental::Detector* detector = m_detVolSvc->detector().get();

    Acts::Experimental::DetectorNavigator::Config navCfg;
    navCfg.detector = detector;
    navCfg.resolvePassive = true;

    auto bField = std::make_shared<ATLASMagneticFieldWrapper>();
    auto bfieldCache = bField->makeCache(mfContext);
    auto stepper = Stepper(bField);
    

    Acts::Experimental::DetectorNavigator navigator(navCfg, Acts::getDefaultLogger("DetectorNavigator", ActsTrk::actsLevelVector(msgLevel())));               
    Acts::Propagator<Stepper, Acts::Experimental::DetectorNavigator> propagator(
        std::move(stepper), std::move(navigator),
        Acts::getDefaultLogger("Propagator", ActsTrk::actsLevelVector(msgLevel())));

    
    for(const xAOD::TruthParticle* truthParticle : *truthParticles){
        
        ATH_MSG_DEBUG("Consider truth particle "<<truthParticle->pt()<<", "<<truthParticle->p4().P()<<", "<<truthParticle->eta()<<", "<<truthParticle->phi()
                        <<", pdgId: "<<truthParticle->pdgId()<<", status: "<< HepMC::status(truthParticle)
                    <<", unique id:"<< HepMC::uniqueID(truthParticle));
        
            //propagated and truth hits expressed in the local frame of the measurement layer           
        std::vector<PropagatorRecorder> propagatedHits;
        std::vector<std::pair<Identifier, Amg::Vector3D>> truthHits;
        
        //the particle hypothesis
        Acts::ParticleHypothesis particleHypothesis(static_cast<Acts::PdgParticle>(truthParticle->absPdgId()),
        truthParticle->m(),
        Acts::AnyCharge(std::abs(truthParticle->charge())));

        
        std::vector<std::pair<const xAOD::MuonSegment*, std::vector<const xAOD::MuonSimHit*>>> muonSegmentWithSimHits;
        for (const auto& truthSegLink : segAcc(*truthParticle)){
            const xAOD::MuonSegment* seg{*truthSegLink}; 
            auto unordedHits = MuonR4::getMatchingSimHits(*seg);
            std::vector<const xAOD::MuonSimHit*> muonSimHits{unordedHits.begin(), unordedHits.end()};           
            ATH_MSG_VERBOSE("Segment at : "<<Amg::toString(seg->position())<<" with Sim Hits: "<<unordedHits.size());
            muonSegmentWithSimHits.emplace_back(seg,muonSimHits);
        }
        const auto& particle = truthParticle->p4();
        m_truthPt = particle.Pt();
        m_truthP = particle.P();
        
        const xAOD::TruthVertex* prodVertex = truthParticle->prodVtx();
        Amg::Vector3D prodPos = prodVertex ? Amg::Vector3D(prodVertex->x(), prodVertex->y(), prodVertex->z()) 
                                            : Amg::Vector3D::Zero();
        Amg::Vector3D prodDir{Amg::dirFromAngles(particle.Phi(), particle.Theta())};

        //position, direction and momentum at the start of the propagation
        Amg::Vector3D startPropPos{prodPos};
        Amg::Vector3D startPropDir{prodDir};
        double startPropP{particle.P()};
        ATH_MSG_VERBOSE("Magnetic field at the production vertex: "<<Amg::toString(bField->getField(prodPos, bfieldCache).value())<<"at position "<<Amg::toString(prodPos));
        for (double r =0; r<=2.*Gaudi::Units::m; r+=10.*Gaudi::Units::cm) {
            Amg::Vector3D magFieldPos{startPropPos.x()+r, startPropPos.y()+r, startPropPos.z()};
            ATH_MSG_VERBOSE("Magnetic Field in the ID is: "<<Amg::toString(bField->getField(magFieldPos, bfieldCache).value()));
            
        }
        
        
        
        
        ATH_MSG_VERBOSE("Production vertex at "<<Amg::toString(prodPos)<<" with direction "<<Amg::toString(prodDir)<<" and momentum "<<startPropP);
        

        if(m_startFromFirstHit){
            if (muonSegmentWithSimHits.empty()) {
                continue;
            }
            //sort the segments by the radial distance
            std::ranges::sort(muonSegmentWithSimHits,
                    [](const auto& a, const auto& b) {
                        return a.first->position().perp() < b.first->position().perp(); 
            });
        
            //get the simHits from the first segment - from sorted segments
            std::vector<const xAOD::MuonSimHit*>& muonSimHits = muonSegmentWithSimHits.front().second;
            if (msgLvl(MSG::VERBOSE)){
                for(const auto& simHit: muonSimHits){
                    ATH_MSG_VERBOSE("Sim Hit of first segment at global position: "<<Amg::toString(toGlobalTrf(*gctx, simHit->identify())*xAOD::toEigen(simHit->localPosition())));
                }
            }

            //sort the simhits to define starting propagation position
            std::ranges::sort(muonSimHits, 
                        [this, &gctx](const xAOD::MuonSimHit* hit1, const xAOD::MuonSimHit* hit2){
                        const Amg::Vector3D globalPos1 = toGlobalTrf(*gctx,hit1->identify())*xAOD::toEigen(hit1->localPosition());
                        const Amg::Vector3D globalPos2 = toGlobalTrf(*gctx,hit2->identify())*xAOD::toEigen(hit2->localPosition());
                        return globalPos1.norm()<globalPos2.norm();
            });
            if (msgLvl(MSG::VERBOSE)){
            ATH_MSG_VERBOSE("After sorting..");
            for(const auto& simHit: muonSimHits){
                ATH_MSG_VERBOSE("Sim Hit of first segment at position: "<<Amg::toString(toGlobalTrf(*gctx, simHit->identify())*xAOD::toEigen(simHit->localPosition())));

            }
            }
        
            startPropPos = toGlobalTrf(*gctx, muonSimHits.front()->identify())*xAOD::toEigen(muonSimHits.front()->localPosition());
            startPropDir = toGlobalTrf(*gctx, muonSimHits.front()->identify()).linear()*xAOD::toEigen(muonSimHits.front()->localDirection());
            startPropP = muonSimHits.front()->kineticEnergy();
            ATH_MSG_VERBOSE("Kinetic Energy from the simHit: "<<muonSimHits.front()->kineticEnergy() / Gaudi::Units::GeV<<" and mass: "<<muonSimHits.front()->mass()<<" and energy deposit: "<<muonSimHits.front()->energyDeposit() / Gaudi::Units::eV);
        }

        ATH_MSG_VERBOSE("Starting propagation from"<<Amg::toString(startPropPos)<<" with direction"
                        <<Amg::toString(startPropDir)<<" and momentum "<<startPropP);   	


	Acts::BoundTrackParameters start = Acts::BoundTrackParameters::createCurvilinear(
            Acts::VectorHelpers::makeVector4(startPropPos, 0.), startPropDir,
            truthParticle->charge() / startPropP,
            std::nullopt,
            particleHypothesis);

        
        Trk::CurvilinearParameters atlasStart(startPropPos, startPropP*startPropDir, truthParticle->charge());

        ATH_MSG_DEBUG("start propagating here");
        auto result = propagator.propagate(start, options);
        const Acts::detail::SteppingLogger::result_type state = result.value().get<Acts::detail::SteppingLogger::result_type>();
        const Acts::MaterialInteractor::result_type material = result.value().get<Acts::MaterialInteractor::result_type>();
        ATH_MSG_DEBUG("Material interactions: " << material.materialInteractions.size());
        ATH_MSG_DEBUG("material in x0: " << material.materialInX0);
        ATH_MSG_DEBUG("material in L0: " << material.materialInL0);
        std::vector<Acts::MaterialInteraction> interactions = material.materialInteractions;
        for(const Acts::MaterialInteraction& interaction : interactions){
                ATH_MSG_DEBUG("Path Correction: " << interaction.pathCorrection);
                ATH_MSG_DEBUG("Momentum change: " << interaction.deltaP);
        }
        // loop through the states drawing each surface or portal
        if(m_drawEvent){
            Acts::ObjVisualization3D helper{};
            for (const auto& s : state.steps) {
                if(s.surface) Acts::GeometryView3D::drawSurface(helper, *s.surface, geoContext);
            }
            helper.write(std::format("event_{:}_{:}_Hits.obj", ctx.eventID().event_number(), truthParticle->index()));
        }
    
        ATH_MSG_VERBOSE("Number of propagated steps : " << state.steps.size());
        m_propSteps = state.steps.size();
        m_propLength = result.value().pathLength;
        for(const auto& state: state.steps){
            if(!state.surface){
                continue;
            }
            const SurfaceCache* sCache = dynamic_cast<const SurfaceCache *>(state.surface->associatedDetectorElement());
            if(!sCache){
                ATH_MSG_VERBOSE("Surface found but it's a portal, continuing..");
                continue;
            }
            const Identifier ID = sCache->identify();
            const Amg::Transform3D toGap{toLocalTrf(*gctx, ID)};
            ATH_MSG_VERBOSE("Identify propagated hit " << m_idHelperSvc->toString(ID) << " with hit at local position " << Amg::toString(toGap*state.position)<<" and global direction "<<Amg::toString(state.momentum.unit()));
            ATH_MSG_VERBOSE("Magnetic field "<<Amg::toString(bField->getField(state.position, bfieldCache).value())<<"at position "<<Amg::toString(state.position));
            PropagatorRecorder newRecord{};
            newRecord.id = ID;
            newRecord.actsPropPos = toGap*state.position;
            newRecord.actsGlobalPos = state.position;
            newRecord.actsPropDir = toGap.linear()*state.momentum.unit();
            newRecord.actsPropabsMomentum = state.momentum.norm();
            newRecord.actsStepSize = state.stepSize.value();

            //define the surface to propagate
            const Trk::Surface& surface = detMgr->getReadoutElement(ID)->surface(ID);

            Trk::ParticleHypothesis particleType = Trk::geantino;
            if(truthParticle->isMuon()){
                particleType = Trk::muon;   
            }
            

            std::unique_ptr<Trk::TrackParameters> atlasPars{m_extrapolator->extrapolate(ctx, atlasStart,
                                                            surface, Trk::alongMomentum,false, 
                                                            particleType, Trk::MaterialUpdateMode::removeNoise)};


            if (atlasPars) {
                newRecord.atlasPropPos =  toGap* atlasPars->position();
                newRecord.atlasPropDir = toGap.linear()*atlasPars->momentum().unit();
                newRecord.atlasPropabsMomentum = atlasPars->momentum().mag();
                newRecord.atlasGlobalPos = atlasPars->position();
            }
            propagatedHits.emplace_back(newRecord);
        }

        std::size_t propHitsSize = propagatedHits.size();

        //fill with the truthHits- id and local position on the measurement layer's frame and look in the propagated hits
        unsigned int nMatchedTruth = 0, nMatchedProp = 0, nTruth{0};
        for (const auto&[segment, simHits] : muonSegmentWithSimHits) {
            for(const xAOD::MuonSimHit* simHit : simHits){
                const Identifier ID = simHit->identify();
                ++nTruth;
                //express the truth hits in the 1st measurement of readout element's local frame - same as the propagated hits
                const Amg::Transform3D localTrf{toLocalTrf(*gctx, simHit->identify())*
                    toGlobalTrf(*gctx,simHit->identify())};
                const Amg::Vector3D localPos = localTrf*xAOD::toEigen(simHit->localPosition());
                const Amg::Vector3D globalPos = toGlobalTrf(*gctx, simHit->identify())*xAOD::toEigen(simHit->localPosition());
                const Amg::Vector3D localDir = localTrf.linear()*xAOD::toEigen(simHit->localDirection());
                m_detId.push_back(ID);
                m_techIdx.push_back(toInt(m_idHelperSvc->technologyIndex(ID)));
                m_gasGapId.push_back(layerHash(ID));

                m_truthLoc.push_back(localPos);
                m_truthDir.push_back(localDir);
                m_truthGlob.push_back(globalPos);
                m_startGlob.push_back(startPropPos);
                ATH_MSG_VERBOSE("Identify truth hit at local position in 1st measurement plane" << Amg::toString(localPos));

                auto it_begin = std::ranges::find_if(propagatedHits,
                                        [this, ID](const auto& propagatedHit) {
                                        return  m_idHelperSvc->detElId(ID) == m_idHelperSvc->detElId(propagatedHit.id) &&
                                                layerHash(ID) == layerHash(propagatedHit.id);
                });
                m_isPropagated.push_back(it_begin != propagatedHits.end());

                if (it_begin == propagatedHits.end()) {
                    m_actsPropLoc.push_back(dummyPos);
                    m_actsPropDir.push_back(dummyPos);
                    m_atlasPropLoc.push_back(dummyPos);
                    m_atlasPropDir.push_back(dummyPos);
                    m_actsPropGlob.push_back(dummyPos);
                    m_atlasPropGlob.push_back(dummyPos);
                    m_actsPropMomentum.push_back(0.);
                    m_atlasPropMomentum.push_back(0.);
                    m_actsStepSize.push_back(0.);
                    continue;
                }
                auto it_end = std::find_if(it_begin, propagatedHits.end(),
                [this, ID](const auto& propagatedHit) {
                    return  m_idHelperSvc->detElId(ID) != m_idHelperSvc->detElId(propagatedHit.id) ||
                            layerHash(ID) != layerHash(propagatedHit.id);
                });
                /// now let's find the closest one
                auto it = std::min_element(it_begin, it_end,
                                       [&localPos](const PropagatorRecorder& a,
                                                   const PropagatorRecorder& b){
                    return (localPos - a.actsPropPos).mag() < (localPos - b.actsPropPos).mag();
                });


                ATH_MSG_DEBUG("Truth hit found in propagated hits: " << m_idHelperSvc->toString(ID));
                m_actsPropLoc.push_back(it->actsPropPos);
                m_actsPropDir.push_back(it->actsPropDir);
                m_actsPropGlob.push_back(it->actsGlobalPos);
                m_atlasPropLoc.push_back(it->atlasPropPos);
                m_atlasPropDir.push_back(it->atlasPropDir);    
                m_atlasPropGlob.push_back(it->atlasGlobalPos); 
                m_actsPropMomentum.push_back(it->actsPropabsMomentum);
                m_atlasPropMomentum.push_back(it->atlasPropabsMomentum);     
                m_actsStepSize.push_back(it->actsStepSize);           
                ++nMatchedTruth;
            }
        }
        m_matchedTruthFraction = 1.f*nMatchedTruth / nTruth;
        m_matchedPropFraction = 1.f*nMatchedProp / propHitsSize;
        m_event = ctx.eventID().event_number();
        m_tree.fill(ctx); 
    }

    return StatusCode::SUCCESS;
}
}  // namespace ActsTrk`

