/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "ActsMuonTrackingGeometryTest.h"

#include "AthenaKernel/IAthRNGSvc.h"
#include "AthenaKernel/RNGWrapper.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GeoPrimitives/GeoPrimitivesHelpers.h"
#include "ActsInterop/LoggerUtils.h"
#include "ActsInterop/UnitConverters.h" 
#include "ActsGeoUtils/SurfaceCache.h"

#include "MuonReadoutGeometry/MuonReadoutElement.h"
#include "xAODMuon/MuonSegmentContainer.h"
#include "xAODTruth/TruthVertex.h"
#include "xAODTruth/TruthVertex.h"
#include "MuonTruthHelpers/MuonSimHitHelpers.h"
#include "TruthUtils/HepMCHelpers.h"



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

namespace {
using SegLink_t = std::vector<ElementLink<xAOD::MuonSegmentContainer>>;
static const SG::ConstAccessor<SegLink_t> segAcc{"truthSegmentLinks"};
static const Amg::Vector3D dummyVec{100.*Gaudi::Units::m, 100.*Gaudi::Units::m, 100.*Gaudi::Units::m};

struct PropagatorRecorder{
    /// @brief Position obtained by the ACTS propagator
    Amg::Vector3D actsPropPos{dummyVec};
    /// @brief Position obtained by the ACTS propagator in the global frame
    Amg::Vector3D actsGlobalPos{dummyVec};
    /// @brief Direction obtained by the ACTS propgator
    Amg::Vector3D actsPropDir{Amg::Vector3D::Zero()};
    /// @brief Identifier of the propagated hit
    Identifier id{};  
    /// @brief Momentum obtained by the ACTS propagator
    double actsPropabsMomentum{0.};
    /// @brief The step size from the acts propagator
    double actsStepSize{0.};
    /// @brief Distance of the propagated position to the wire in Mdts
    double actsHitWireDist{-1.};
};

}


namespace ActsTrk {


    Amg::Transform3D ActsMuonTrackingGeometryTest::toLocalTrf(const ActsTrk::GeometryContext& gctx, const Identifier& hitId) const {
        const MuonGMR4::MuonReadoutElement* reElement = m_r4DetMgr->getReadoutElement(hitId);    
        const IdentifierHash trfHash = reElement->detectorType() == ActsTrk::DetectorType::Mdt ?
                                    reElement->measurementHash(hitId) : reElement->layerHash(hitId);        
        return reElement->globalToLocalTransform(gctx, trfHash);
    }

    Amg::Transform3D ActsMuonTrackingGeometryTest::toGlobalTrf(const ActsTrk::GeometryContext& gctx, const Identifier& hitId) const {
    const MuonGMR4::MuonReadoutElement* reElement = m_r4DetMgr->getReadoutElement(hitId);
    const IdentifierHash trfHash = reElement->detectorType() == ActsTrk::DetectorType::Mdt ?
                                    reElement->measurementHash(hitId) : reElement->layerHash(hitId);            
    return reElement->localToGlobalTransform(gctx, trfHash);
    }


    IdentifierHash ActsMuonTrackingGeometryTest::layerHash(const Identifier& id) const {
    return m_r4DetMgr->getReadoutElement(id)->layerHash(id);
    }


    StatusCode ActsMuonTrackingGeometryTest::initialize() {
        ATH_CHECK(AthHistogramAlgorithm::initialize());
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_trackingGeometryTool.retrieve());
        ATH_CHECK(m_detMgrKey.initialize());
        ATH_CHECK(m_rndmGenSvc.retrieve());
        ATH_CHECK(m_geoCtxKey.initialize());
        ATH_CHECK(m_fieldCacheCondObjInputKey.initialize());
        ATH_CHECK(m_truthParticleKey.initialize());
        ATH_CHECK(m_truthSegLinkKey.initialize());
        ATH_CHECK(detStore()->retrieve(m_r4DetMgr));

        ATH_CHECK(m_tree.init(this));
        ATH_MSG_INFO("ActsMuonTrackingGeometryTest  successfully initialized");
        return StatusCode::SUCCESS;
    }

    StatusCode ActsMuonTrackingGeometryTest::finalize() {
        ATH_CHECK(m_tree.write());
        return StatusCode::SUCCESS;
    }

    StatusCode ActsMuonTrackingGeometryTest::execute() {

        const EventContext& ctx = Gaudi::Hive::currentContext();

        const ActsTrk::GeometryContext* gctx{nullptr};
        const AtlasFieldCacheCondObj* fieldCondObj{nullptr};
        const MuonGM::MuonDetectorManager* detMgr{nullptr};
        const xAOD::TruthParticleContainer* truthParticles{nullptr};

        ATH_CHECK(SG::get(fieldCondObj, m_fieldCacheCondObjInputKey, ctx));
        ATH_CHECK(SG::get(detMgr, m_detMgrKey, ctx));
        ATH_CHECK(SG::get(truthParticles, m_truthParticleKey, ctx));
        ATH_CHECK(SG::get(gctx, m_geoCtxKey, ctx));


        const Acts::MagneticFieldContext mfContext = Acts::MagneticFieldContext(fieldCondObj);

        auto anygctx = gctx->context();

        //Get the tracking geometry
        auto trackingGeometry = m_trackingGeometryTool->trackingGeometry();

        if (!trackingGeometry) {
            ATH_MSG_ERROR("Failed to retrieve the tracking geometry");
            return StatusCode::FAILURE;
        }

        //Configure the ACTS propagator with the navigator and the stepper
        using Stepper = Acts::EigenStepper<>;
        using Navigator = Acts::Navigator;
        using Propagator = Acts::Propagator<Stepper,Navigator>;
        using ActorList = Acts::ActorList<Acts::detail::SteppingLogger, Acts::MaterialInteractor, Acts::EndOfWorldReached>;
        using PropagatorOptions = Propagator::Options<ActorList>;

        Navigator::Config navCfg;
        navCfg.trackingGeometry = trackingGeometry;
        navCfg.resolveSensitive = true;
        navCfg.resolveMaterial= false;
        navCfg.resolvePassive = true;        

        Navigator navigator(navCfg, Acts::getDefaultLogger("Navigator", ActsTrk::actsLevelVector(msgLevel())));

        auto bField = std::make_shared<ATLASMagneticFieldWrapper>();
        auto bfieldCache = bField->makeCache(mfContext);

        auto stepper = Stepper(bField);

        PropagatorOptions options(anygctx, mfContext);
        options.pathLimit = m_pathLimit;
        options.stepping.maxStepSize = m_maxStepSize;
        options.stepping.stepTolerance = m_stepTolerance;
        options.maxSteps = m_maxSteps;
        options.maxTargetSkipping = m_maxTargetSkipping;


        //switch off material interactions
        auto& materialInteractor = options.actorList.get<Acts::MaterialInteractor>();
        materialInteractor.energyLoss = false;
        materialInteractor.multipleScattering = false;
        materialInteractor.recordInteractions = false;

        Propagator propagator(std::move(stepper), std::move(navigator),  
        Acts::getDefaultLogger("Propagator", ActsTrk::actsLevelVector(msgLevel())));

        for(const xAOD::TruthParticle* truthParticle : *truthParticles) {

            ATH_MSG_DEBUG("Processing truth particle with PDG ID: " << truthParticle->pdgId() << " ,pT: "
            << truthParticle->pt() << " , p: " << truthParticle->p4().P() << ", eta: " << truthParticle->eta() << " , phi: " << truthParticle->phi());

            //select only muons or geantinos particles
            if(std::abs(truthParticle->pdgId()) != 13 && std::abs(truthParticle->pdgId()) != 998) {
                ATH_MSG_VERBOSE("Skipping truth particle with PDG ID: " << truthParticle->pdgId()<<" only muons or charged geantinos are being processed");
                continue;
            }
           
            Acts::ParticleHypothesis actsParticleHypothesis = truthParticle->pdgId() == 998 ? 
                Acts::ParticleHypothesis::chargedGeantino() : Acts::ParticleHypothesis::muon();


            //take the truth segments witht he sim hits
            std::vector<std::pair<const xAOD::MuonSegment*, std::vector<const xAOD::MuonSimHit*>>> muonSegmentWithSimHits;
            const SegLink_t& segLink = segAcc(*truthParticle);
            if (segLink.empty()) {
                ATH_MSG_WARNING("No segment link found for truth particle with PDG ID: " << truthParticle->pdgId());
                continue;
            }
            for(const auto& truthSegLink: segLink) {
                 const xAOD::MuonSegment* seg{*truthSegLink}; 
                 if(!seg){
                    continue;
                 }
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
            ATH_MSG_VERBOSE("Truth particle produced at " << Amg::toString(prodPos) << " with direction " << Amg::toString(prodDir));

            //position, direction and momentum at the start of the propagation
            Amg::Vector3D startPropPos{prodPos};
            Amg::Vector3D startPropDir{prodDir};            
            double startPropP{energyToActs(particle.P())};

            if(m_startFromFirstHit) {

            if(muonSegmentWithSimHits.empty()) {
                ATH_MSG_DEBUG("No segments found for truth particle with PDG ID: " << truthParticle->pdgId());
                continue;
            }
            // sort the segments by their radial distance
            std::ranges::sort(muonSegmentWithSimHits, 
                [](const auto& a, const auto& b) {
                    return a.first->position().perp() < b.first->position().perp();
                });
            
            
            //sort the sim hits of the first segment to get the starting propagation position as the first one
            std::vector<const xAOD::MuonSimHit*>& muonSimHits = muonSegmentWithSimHits.front().second;
            std::ranges::sort(muonSimHits, 
                        [this, &gctx](const xAOD::MuonSimHit* hit1, const xAOD::MuonSimHit* hit2){
                        const Amg::Vector3D globalPos1 = toGlobalTrf(*gctx,hit1->identify())*xAOD::toEigen(hit1->localPosition());
                        const Amg::Vector3D globalPos2 = toGlobalTrf(*gctx,hit2->identify())*xAOD::toEigen(hit2->localPosition());
                        return globalPos1.norm()<globalPos2.norm();
            });
             
            ATH_MSG_VERBOSE("After sorting..");
            for(const auto& simHit: muonSimHits){
                ATH_MSG_VERBOSE("Sim Hit of first segment at position: "<<Amg::toString(toGlobalTrf(*gctx, simHit->identify())*xAOD::toEigen(simHit->localPosition())));

            }


            startPropPos = toGlobalTrf(*gctx, muonSimHits.front()->identify())*xAOD::toEigen(muonSimHits.front()->localPosition());
            startPropDir = toGlobalTrf(*gctx, muonSimHits.front()->identify()).linear()*xAOD::toEigen(muonSimHits.front()->localDirection());
            startPropP = energyToActs(muonSimHits.front()->kineticEnergy());

           
            ATH_MSG_VERBOSE("Kinetic Energy from the simHit: "<<startPropP / Gaudi::Units::GeV<<" and mass: "<<muonSimHits.front()->mass()<<" and energy deposit: "<<muonSimHits.front()->energyDeposit() / Gaudi::Units::eV);
            

            }

            ATH_MSG_VERBOSE("Starting propagation from"<<Amg::toString(startPropPos)<<" with direction"
                        <<Amg::toString(startPropDir)<<" and momentum "<<startPropP);   	

            

            Acts::BoundTrackParameters start = Acts::BoundTrackParameters::createCurvilinear(
            Acts::VectorHelpers::makeVector4(startPropPos, 0.), startPropDir,
            truthParticle->charge() / startPropP,
            std::nullopt,
            actsParticleHypothesis);

            ATH_MSG_DEBUG("start propagating here");
            auto result = propagator.propagate(start, options);
            const Acts::detail::SteppingLogger::result_type state = result.value().get<Acts::detail::SteppingLogger::result_type>();
            const Acts::MaterialInteractor::result_type material = result.value().get<Acts::MaterialInteractor::result_type>();

            m_propSteps = state.steps.size();
            ATH_MSG_DEBUG("Number of propagated steps : " << m_propSteps);
            m_propLength = result.value().pathLength;
            std::vector<PropagatorRecorder> propagatedHits;

            for(const auto& step : state.steps) {
                if(!step.surface) {
                    continue;
                }

                const SurfaceCache* sCache = dynamic_cast<const SurfaceCache *>(step.surface->surfacePlacement());
                if(!sCache) {
                    ATH_MSG_VERBOSE("Surface found but it's a portal, continuing..");
                    continue;
                }
                const Identifier ID = sCache->identify();
                const Amg::Transform3D toGap{toLocalTrf(*gctx, ID)};
                ATH_MSG_VERBOSE("Identify propagated hit " << m_idHelperSvc->toString(ID) << " with hit at local position " << Amg::toString(toGap*step.position)<<" and global direction "<<Amg::toString(step.momentum.unit()));
                
                
                PropagatorRecorder newRecord{};
                newRecord.id = ID;
                newRecord.actsPropPos = toGap*step.position;
                newRecord.actsGlobalPos = step.position;
                newRecord.actsPropDir = toGap.linear()*step.momentum.unit();
                newRecord.actsPropabsMomentum = step.momentum.norm();
                newRecord.actsStepSize = step.stepSize.value();
                //calculate the distance of the propagated hit to the wire in case of MDTs
      
                if(m_idHelperSvc->isMdt(ID)){
                    //get the surface 
                    const auto* tubeSurf = dynamic_cast<const Acts::StrawSurface*>(&sCache->surface());
                    if(tubeSurf){
                    Amg::Vector3D wirePos = toGap*tubeSurf->center(anygctx);
                    Amg::Vector3D wireDir = toGap.linear()*tubeSurf->lineDirection(anygctx);
                    Amg::Vector3D localPropHit = newRecord.actsPropPos;
                    Amg::Vector3D dirPropHit = newRecord.actsPropDir;
                    double distToWire = Amg::lineDistance(wirePos, wireDir, localPropHit, dirPropHit);
                    newRecord.actsHitWireDist = distToWire;
                    }

                }

                propagatedHits.emplace_back(newRecord);

            }


            for (const auto&[segment, simHits] : muonSegmentWithSimHits) {
                for(const xAOD::MuonSimHit* simHit : simHits){

                    const Identifier ID = simHit->identify();

                    const Amg::Vector3D localPos = xAOD::toEigen(simHit->localPosition());

                    const Amg::Vector3D globalPos = toGlobalTrf(*gctx, simHit->identify())*xAOD::toEigen(simHit->localPosition());
                    const Amg::Vector3D localDir = xAOD::toEigen(simHit->localDirection());
                    
                    if(m_r4DetMgr->getReadoutElement(ID)->detectorType() == ActsTrk::DetectorType::sTgc && localPos.z() != 0.0){
                        continue;
                    
                    }
                
                    m_detId.push_back(ID);
                    m_techIdx.push_back(toInt(m_idHelperSvc->technologyIndex(ID)));
                    m_gasGapId.push_back(layerHash(ID));

                    m_truthLoc.push_back(localPos);
                    m_truthDir.push_back(localDir);
                    m_truthGlob.push_back(globalPos);
                    m_startGlob.push_back(startPropPos);
                    ATH_MSG_DEBUG("Truth hit with ID: " << m_idHelperSvc->toString(ID)
                    << " at local position: " << Amg::toString(localPos)
                    << " and global position: " << Amg::toString(globalPos)
                    << " and direction: " << Amg::toString(localDir));        
           
                    //get the matching propagated hits
                    auto it_begin = std::ranges::find_if(propagatedHits,
                                        [this, ID](const auto& propagatedHit) {
                                        return  m_idHelperSvc->detElId(ID) == m_idHelperSvc->detElId(propagatedHit.id) &&
                                                layerHash(ID) == layerHash(propagatedHit.id);
                    });
                    m_isPropagated.push_back(it_begin != propagatedHits.end());

                    if(it_begin == propagatedHits.end()){
                        m_actsPropLoc.push_back(dummyVec);
                        Amg::Vector3D zero = Amg::Vector3D::Zero();
                        m_actsPropDir.push_back(zero);
                        m_actsPropGlob.push_back(dummyVec);
                        m_actsPropabsMomentum.push_back(0.);
                        m_actsStepSize.push_back(0.);
                        m_actsHitWireDist.push_back(-1.);
                        continue;
                    }

                    //find - if any - propagated hits on the same layer 
                     auto it_end = std::find_if(it_begin, propagatedHits.end(),
                    [this, ID](const auto& propagatedHit) {
                        return  m_idHelperSvc->detElId(ID) != m_idHelperSvc->detElId(propagatedHit.id) ||
                                layerHash(ID) != layerHash(propagatedHit.id);
                    });

                    /// find the closest propagated hit to the truth hit
                    auto it = std::min_element(it_begin, it_end,
                                        [&localPos](const PropagatorRecorder& a,
                                                    const PropagatorRecorder& b){
                        return (localPos - a.actsPropPos).mag() < (localPos - b.actsPropPos).mag();
                    });

                    ATH_MSG_DEBUG("Found propagated hit with ID: " << m_idHelperSvc->toString(it->id)
                    << " at local position: " << Amg::toString(it->actsPropPos)
                    << " and global position: " << Amg::toString(it->actsGlobalPos)
                    << " and direction: " << Amg::toString(it->actsPropDir));

                    m_actsPropLoc.push_back(it->actsPropPos);
                    m_actsPropDir.push_back(it->actsPropDir);
                    m_actsPropGlob.push_back(it->actsGlobalPos);
                    m_actsPropabsMomentum.push_back(it->actsPropabsMomentum);
                    m_actsStepSize.push_back(it->actsStepSize);
                    m_actsHitWireDist.push_back(it->actsHitWireDist);

              
                }
            }

        m_event = ctx.eventID().event_number();
        m_tree.fill(ctx); 


        }


        return StatusCode::SUCCESS;
    }

}

