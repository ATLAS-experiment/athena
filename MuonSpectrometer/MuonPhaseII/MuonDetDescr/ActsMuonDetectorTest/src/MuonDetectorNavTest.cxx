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
#include "Acts/Utilities/VectorHelpers.hpp"
#include "Acts/Visualization/ObjVisualization3D.hpp"
#include "Acts/Visualization/GeometryView3D.hpp"

#include "xAODTruth/TruthVertex.h"

#include <fstream>

using namespace Acts::UnitLiterals;

//Struct that is passed as an actor to ACTS that records all state information during propagation
//Contains the information at the following link: https://github.com/acts-project/acts/blob/main/Core/include/Acts/Navigation/NavigationState.hpp
struct StateRecorder {
	using result_type = std::vector<Acts::Experimental::DetectorNavigator::State>;

	template <typename propagator_state_t, typename stepper_t, typename navigator_t>
	void act(propagator_state_t& state, const stepper_t& /*stepper*/,
			const navigator_t& /*navigator*/, result_type& result,
			const Acts::Logger& /*logger*/) const {

		result.push_back(state.navigation);
	}

	template <typename propagator_state_t, typename stepper_t, typename navigator_t>
	bool checkAbort(propagator_state_t& /*state*/, const stepper_t& /*stepper*/,
					const navigator_t& /*navigator*/, result_type& /*result*/,
					const Acts::Logger& /*logger*/) const {
		return false;
	}
};

//Function to write out every step taken during propagation to obj format for visualization
void writeStepsToObj(const StateRecorder::result_type& states, const size_t& eventNumber){
	std::ofstream os("event_" + std::to_string(eventNumber) + "_Steps.obj");
	size_t vCounter = 0;
	//Require at least 3 steps to draw a line
	if(states.size() > 2){
		++vCounter;
		for(const auto& state : states){
			os << "v " << state.position.x() << " " << state.position.y() << " " << state.position.z() << '\n';
		}
		std::size_t vBreak = vCounter + states.size() - 1;
		for(; vCounter < vBreak; ++vCounter){
			os << "l " << vCounter << " " << vCounter + 1 << '\n';
		}
	}
}

namespace ActsTrk {
	MuonDetectorNavTest::MuonDetectorNavTest(const std::string& name, ISvcLocator* pSvcLocator):
		AthHistogramAlgorithm(name, pSvcLocator){}

	Amg::Transform3D MuonDetectorNavTest::toGlobalTrf(const ActsGeometryContext& gctx, const Identifier& hitId) const {
		const MuonGMR4::MuonReadoutElement* reElement = m_r4DetMgr->getReadoutElement(hitId);
		const IdentifierHash trfHash = reElement->detectorType() == ActsTrk::DetectorType::Mdt ?
									   reElement->measurementHash(hitId) : reElement->layerHash(hitId);            
		return reElement->localToGlobalTrans(gctx, trfHash);
	}

	StatusCode MuonDetectorNavTest::initialize() {
		ATH_CHECK(m_idHelperSvc.retrieve());
		ATH_CHECK(m_geoCtxKey.initialize());
		ATH_CHECK(m_fieldCacheCondObjInputKey.initialize());
		ATH_CHECK(m_truthParticleKey.initialize());
		ATH_CHECK(m_inSimHitKeys.initialize());
		ATH_CHECK(m_detVolSvc.retrieve());
		ATH_CHECK(detStore()->retrieve(m_r4DetMgr));
		ATH_CHECK(m_tree.init(this));
		ATH_CHECK(book(TH1D("distanceBetweenHits", "distanceBetweenHits", 250, 0., 500.), "MuonNavigationTestR4", "MuonNavigationTestR4"));

		ATH_MSG_VERBOSE("Successfully initialized MuonDetectorNavTest");
		return StatusCode::SUCCESS;
	}

	StatusCode MuonDetectorNavTest::finalize() {
		ATH_MSG_VERBOSE("Finalizing MuonDetectorNavTest");
		ATH_CHECK(m_tree.write());
		hist("distanceBetweenHits")->GetXaxis()->SetTitle("Distance between hits [mm]");
		return StatusCode::SUCCESS;
	}

	StatusCode MuonDetectorNavTest::execute() {
		ATH_MSG_INFO("Executing MuonDetectorNavTest");
		const EventContext& ctx{Gaudi::Hive::currentContext()};
		SG::ReadHandle<ActsGeometryContext> gctx{m_geoCtxKey, ctx};
		SG::ReadHandle<xAOD::TruthParticleContainer> truthParticles{m_truthParticleKey, ctx};
		SG::ReadCondHandle<AtlasFieldCacheCondObj> magFieldHandle{m_fieldCacheCondObjInputKey, ctx};
		if (!gctx.isValid() or !truthParticles.isValid() or !magFieldHandle.isValid()) {
			ATH_MSG_FATAL("Failed to retrieve either the geometry context, truth particles or magnetic field");
			return StatusCode::FAILURE;
		}
		auto simHitCollections = m_inSimHitKeys.makeHandles(ctx);
		Acts::GeometryContext geoContext = gctx->context();
		Acts::MagneticFieldContext mfContext = Acts::MagneticFieldContext(*magFieldHandle);

		std::vector<std::pair<Identifier, Amg::Vector3D>> propagatedHits;
		std::vector<std::pair<Identifier, Amg::Vector3D>> propagatedHitsGlobal;
		std::vector<std::pair<Identifier, Amg::Vector3D>> truthHits;

		for(auto& simHitCollection : simHitCollections){
			for(const xAOD::MuonSimHit* simHit : *simHitCollection){
				if(simHit->pdgId() != 13) continue;
				const Identifier ID = simHit->identify();
				const Amg::Vector3D localPos = xAOD::toEigen(simHit->localPosition());
				// const Amg::Vector3D globalDir = toGlobalTrf(*gctx, ID).rotation() * xAOD::toEigen(simHit->localDirection());
				ATH_MSG_DEBUG("Local direction for hit ID " << m_idHelperSvc->toString(ID) << ": " << Amg::toString(xAOD::toEigen(simHit->localDirection())));
				truthHits.emplace_back(ID, localPos);
			}
		}

		using Stepper = Acts::EigenStepper<>;
		using Navigator = Acts::Experimental::DetectorNavigator;
		using Propagator = Acts::Propagator<Stepper,Navigator>;
		using ActorList = Acts::ActorList<StateRecorder, Acts::MaterialInteractor, Acts::EndOfWorldReached>;
		using PropagatorOptions = Propagator::Options<ActorList>;

		auto bField = std::make_shared<ATLASMagneticFieldWrapper>();
		auto stepper = Stepper(std::move(bField));

		PropagatorOptions options(geoContext, mfContext);

		options.pathLimit = 23_m;
		options.stepping.maxStepSize = 0.5_m;

		//Configure the material collector
		auto& materialInteractor = options.actorList.get<Acts::MaterialInteractor>();
		materialInteractor.energyLoss = true;
		materialInteractor.multipleScattering = true;
		materialInteractor.recordInteractions = true;
		
		const Acts::Experimental::Detector* detector = m_detVolSvc->detector().get();
		for(const auto truthParticle : *truthParticles){
			//Require that we only propagate on muons, and of status 1
			if(truthParticle->status() == 1 and truthParticle->pdgId() == 13){
				const auto& particle = truthParticle->p4();
				const auto& prodVertex = (!truthParticle->hasProdVtx()) ? nullptr : truthParticle->prodVtx();
				double x = prodVertex ? prodVertex->x() : 0.;
				double y = prodVertex ? prodVertex->y() : 0.;
				double z = prodVertex ? prodVertex->z() : 0.;

				Acts::CurvilinearTrackParameters start(
					Acts::VectorHelpers::makeVector4(Acts::Vector3(x, y, z), 0.),
					Acts::Vector3(particle.Px(), particle.Py(), particle.Pz()).normalized(),
					truthParticle->charge() / particle.P(),
					std::nullopt,
					Acts::ParticleHypothesis::muon());

				Acts::Experimental::DetectorNavigator::Config navCfg;
				navCfg.detector = detector;
				navCfg.resolvePassive = true;

				Acts::Experimental::DetectorNavigator navigator(navCfg, Acts::getDefaultLogger("DetectorNavigator", Acts::Logging::Level::INFO));
				Acts::Propagator<Stepper, Acts::Experimental::DetectorNavigator> propagator(
					std::move(stepper), std::move(navigator),
					Acts::getDefaultLogger("Propagator", Acts::Logging::Level::INFO));

				auto result = propagator.propagate(start, options).value();
				const StateRecorder::result_type state = result.get<StateRecorder::result_type>();
				const Acts::MaterialInteractor::result_type material = result.get<Acts::MaterialInteractor::result_type>();
				ATH_MSG_DEBUG("Material interactions: " << material.materialInteractions.size());
				ATH_MSG_DEBUG("material in x0: " << material.materialInX0);
				ATH_MSG_DEBUG("material in L0: " << material.materialInL0);
				std::vector<Acts::MaterialInteraction> interactions = material.materialInteractions;
				for(const Acts::MaterialInteraction& interaction : interactions){
					ATH_MSG_DEBUG("Path Correction: " << interaction.pathCorrection);
					ATH_MSG_DEBUG("Momentum change: " << interaction.deltaP);
				}
				// loop through the states drawing each surface or portal
				Acts::ObjVisualization3D helper{};
				for (const auto& s : state) {
					if(s.currentSurface) Acts::GeometryView3D::drawSurface(helper, *s.currentSurface, geoContext);
				}
				helper.write("event_" + std::to_string(ctx.evt() + 1) +"_Hits.obj");
				helper.clear();
				writeStepsToObj(state, ctx.evt() + 1);
				ATH_MSG_VERBOSE("Number of propagated steps : " << state.size());
				for(const auto& state: state){
					if(state.currentSurface){
						const SurfaceCache* sCache = dynamic_cast<const SurfaceCache *>(state.currentSurface->associatedDetectorElement());
						if(!sCache){
							ATH_MSG_VERBOSE("Surface found but it's a portal, continuing..");
							continue;
						}
						const Identifier ID = sCache->identify();
						const Amg::Transform3D localTrf = sCache->transform(geoContext).inverse();
						const Amg::Vector3D localPos{localTrf * state.position};
						ATH_MSG_DEBUG("Identify hit " << m_idHelperSvc->toString(ID) << " with hit at " << Amg::toString(localPos));
						propagatedHits.emplace_back(ID, localPos);
						propagatedHitsGlobal.emplace_back(ID, state.position);
					}
				}
			}
		}
		//compare the hits
		for(const auto& truthHit : truthHits){
			bool hitFound = false;
			for(const auto& propagatedHit : propagatedHits){
				if(truthHit.first == propagatedHit.first){
					hitFound = true;
					break;
				}
			}
			if(!hitFound) ATH_MSG_VERBOSE("Truth hit not found, ID: " << m_idHelperSvc->toString(truthHit.first) << " at " << Amg::toString(truthHit.second));
			
		}
		//Check if all propagated hits are found
		for(const auto& propagatedHit : propagatedHits){
			bool hitFound = false;
			for(const auto& truthHit : truthHits){
				if(truthHit.first == propagatedHit.first){
					hitFound = true;
					break;
				}
			}
			if(!hitFound){
				ATH_MSG_VERBOSE("Propagated hit not found, ID: " << m_idHelperSvc->toString(propagatedHit.first) << " at " << Amg::toString(propagatedHit.second));
			}
		}

		//For one final check, put all hits into the global frame, and find which hits are closest to one another
		for(const auto& propagatedHit: propagatedHitsGlobal){
			double minDistance = std::numeric_limits<double>::max();
			Identifier closestID{};
			for(const auto& truthHit: truthHits){
				const double distance = (propagatedHit.second - toGlobalTrf(*gctx, truthHit.first) * truthHit.second).norm();
				if(distance < minDistance){
					minDistance = distance;
					closestID = truthHit.first;
				}
			}
			ATH_MSG_VERBOSE("Closest hit to propagated hit " << m_idHelperSvc->toString(propagatedHit.first) << " is " << m_idHelperSvc->toString(closestID) << " with distance " << minDistance);
			m_distanceBetweenHits = minDistance;
			m_propagatedIdentifier = m_idHelperSvc->toString(propagatedHit.first);
			m_truthIdentifier = m_idHelperSvc->toString(closestID);
			hist("distanceBetweenHits")->Fill(minDistance);
			m_tree.fill(ctx);
		}

		return StatusCode::SUCCESS;
	}
}
