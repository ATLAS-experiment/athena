/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4ATLASTOOLS_ACTSFATRASG4TOOL_H
#define G4ATLASTOOLS_ACTSFATRASG4TOOL_H

/**
  @class ActsFatrasG4Tool

   Tool for ACTS Fatras G4.
   
  @author marilena.bandieramonte@cern.ch, rui.wang@cern.ch, firdaus.soberi@cern.ch
*/

// interfaces
#include "G4AtlasInterfaces/IPhysicsInitialization.h"
#include "G4AtlasInterfaces/IActsFatrasG4Tool.h"

// Gaudi, StoreGate and Athena
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/EventContext.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaKernel/SlotSpecificObj.h"
#include "AthenaKernel/IAthRNGSvc.h"
#include "AthenaKernel/RNGWrapper.h"
#include "CxxUtils/checker_macros.h"
#include "CxxUtils/ConcurrentMap.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

// ATLAS (for the hits)
#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"
#include "InDetSimEvent/SiHitCollection.h"
#include "InDetSimEvent/SiHit.h"

// ACTS
#include "Acts/Utilities/UnitVectors.hpp"
#include "ActsInterop/Logger.h"
#include "ActsInterop/UnitConverters.h"
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Geometry/GeometryContext.hpp"
#include "Acts/MagneticField/MagneticFieldContext.hpp"
#include "Acts/EventData/BoundTrackParameters.hpp"
#include "Acts/Propagator/Navigator.hpp"
#include "Acts/Propagator/EigenStepper.hpp"
#include "Acts/Propagator/EigenStepperDefaultExtension.hpp"
#include "Acts/Propagator/StraightLineStepper.hpp"
#include "Acts/Propagator/detail/SteppingLogger.hpp"
#include "Acts/Propagator/ActorList.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Definitions/ParticleData.hpp"
#include <Acts/Definitions/Algebra.hpp>
// ActsFatras
#include "ActsFatras/EventData/Particle.hpp"
#include "ActsFatras/EventData/Hit.hpp"
#include "ActsFatras/EventData/GenerationProcess.hpp"
#include "ActsFatras/Kernel/InteractionList.hpp"
#include "ActsFatras/Kernel/SingleParticleSimulation.hpp"
#include "ActsFatras/Kernel/MultiParticleSimulation.hpp"
#include "ActsFatras/Kernel/SingleParticleSimulationResult.hpp"
#include "ActsFatras/Kernel/detail/SimulationActor.hpp"
#include "ActsFatras/Physics/Decay/NoDecay.hpp"
#include "ActsFatras/Physics/StandardInteractions.hpp"
#include "ActsFatras/Physics/ElectroMagnetic/PhotonConversion.hpp"
#include "ActsFatras/Selectors/SurfaceSelectors.hpp"

// Tracking
#include "ActsGeometryInterfaces/ITrackingGeometrySvc.h"
#include "ActsGeometry/ATLASMagneticFieldWrapper.h"
#include "ActsEvent/ContextUtility.h"

// Geant4
#include "G4FastTrack.hh"
#include "G4FastStep.hh"
#include "G4Track.hh"

// C++ STL
#include <utility>
#include <array>
#include <string>
#include <algorithm>
#include <cassert>
#include <vector>
#include <map>

class ActsFatrasG4Tool : virtual public extends<AthAlgTool, IActsFatrasG4Tool, IPhysicsInitializationTool>
{
  public:
    using base_class::base_class;
    virtual ~ActsFatrasG4Tool () = default;

    /** AlgTool initialize method */
    virtual StatusCode initialize() override;
    /** AlgTool finalize method */
    virtual StatusCode finalize() override;
    /** AlgTool initializePhysics method */
    StatusCode initializePhysics() override;

    /** create ActsFatras track, exposed via interface */
    virtual void simulateFatrasTrack(const G4FastTrack& fastTrack, G4FastStep& fastStep) override;

    /** for saving si hits caches, expose to the interface */
    virtual const std::vector<SiHit>& getPixelHitsCache(const EventContext& ctx) const override;
    virtual const std::vector<SiHit>& getSCTHitsCache(const EventContext& ctx) const override;
    virtual void clearCaches(const EventContext& ctx) const override;

  private:
    
    /** @brief Context provider for geometry, magnetic field and calibration contexts */
    ActsTrk::ContextUtility m_ctxProvider{this};
    
    /// Simple struct to select surfaces where hits should be generated.
    /// Check if the surface should be used based on sensitive boolean.
    struct HitSurfaceSelector {
        bool operator()(const Acts::Surface &surface) const {
            return surface.isSensitive();
        }
    };

    // SingleParticleSimulation
    /// Single particle simulation with fixed propagator, interactions, and decay.
    ///
    /// @tparam generator_t random number generator
    /// @tparam interactions_t interaction list
    /// @tparam hit_surface_selector_t selector for hit surfaces
    /// @tparam decay_t decay module
    template <typename propagator_t, typename interactions_t, typename hit_surface_selector_t, typename decay_t>
    struct SingleParticleSimulation {
        /// How and within which geometry to propagate the particle.
        propagator_t propagator;
        /// Decay module.
        decay_t decay;
        /// Interaction list containing the simulated interactions.
        interactions_t interactions;
        /// Selector for surfaces that should generate hits.
        hit_surface_selector_t selectHitSurface;
        /// parameters for propagator options
        double maxStepSize = 3.0; // leght in m
        double maxStep = 1000;
        double maxRungeKuttaStepTrials = 10000;
        double pathLimit = 100.0; // lenght in cm
        bool   loopProtection = true;
        double loopFraction = 0.5;
        double targetTolerance = 0.0001;
        double stepSizeCutOff = 0.;
        // parameters for densEnv propagator options
        double meanEnergyLoss = true;
        bool   includeGgradient = true;
        double momentumCutOff = 0.;

        /// Local logger for debug output.
        std::shared_ptr<const Acts::Logger> localLogger = nullptr;

        /// Alternatively construct the simulator with an external logger.
        SingleParticleSimulation(propagator_t &&propagator_, std::shared_ptr<const Acts::Logger> localLogger_)
            : propagator(propagator_), localLogger(localLogger_) {}

        /// Provide access to the local logger instance, e.g. for logging macros.
        const Acts::Logger &logger() const { return *localLogger; }

        /// Simulate a single particle without secondaries.
        ///
        /// @tparam generator_t is the type of the random number generator
        ///
        /// @param geoCtx is the geometry context to access surface geometries
        /// @param magCtx is the magnetic field context to access field values
        /// @param generator is the random number generator
        /// @param particle is the initial particle state
        /// @returns Simulated particle state, hits, and generated particles.
        template <typename generator_t>
        Acts::Result<ActsFatras::SingleParticleSimulationResult> simulate(const Acts::GeometryContext &geoCtx, 
                                                            const Acts::MagneticFieldContext &magCtx, generator_t &generator,
                                                            const ActsFatras::Particle &particle) const 
        {
            assert(localLogger and "Missing local logger");
            ACTS_VERBOSE("Using ActsFatrasSimTool simulate()");
            // propagator-related additional types
            using SteppingLogger = Acts::detail::SteppingLogger;
            using SimulationActor = ActsFatras::detail::SimulationActor<generator_t, decay_t, interactions_t, hit_surface_selector_t>;
            using Result = typename SimulationActor::result_type;
            using Actions = Acts::ActorList<SteppingLogger, SimulationActor, Acts::EndOfWorldReached>;
            using PropagatorOptions = typename propagator_t::template Options<Actions>;

            // Construct per-call options.
            PropagatorOptions options(geoCtx, magCtx);
            // setup the interactor as part of the propagator options
            auto &actor = options.actorList.template get<SimulationActor>();
            actor.generator = &generator;
            actor.decay = decay;
            actor.interactions = interactions;
            actor.selectHitSurface = selectHitSurface;
            actor.initialParticle = particle;
            // use AnyCharge to be able to handle neutral and charged parameters
            Acts::BoundTrackParameters startPoint = Acts::BoundTrackParameters::createCurvilinear(particle.fourPosition(), 
                                                                                                    particle.direction(),
                                                                                                    particle.qOverP(), std::nullopt, 
                                                                                                    particle.hypothesis()
                                                                                                    );
            options.pathLimit             = pathLimit * Acts::UnitConstants::cm;
            options.loopProtection        = loopProtection;
            options.maxSteps              = maxStep;
            options.stepping.maxStepSize  = maxStepSize * Acts::UnitConstants::m;
            auto result = propagator.propagate(startPoint, options);
            if (not result.ok()) {return result.error();}
            return result.value().template get<Result>();
        }
    };// end of SingleParticleSimulation

    // ===============================
    // Random number generator 
    // ===============================
    using Generator = std::ranlux48;
    // ===============================
    // Setup ActsFatras simulator types
    // ===============================
    // Use default navigator
    using Navigator = Acts::Navigator;
    // propagate charged particles numerically in the B-field
    using ChargedStepper = Acts::EigenStepper<Acts::EigenStepperDefaultExtension>;
    using ChargedPropagator = Acts::Propagator<ChargedStepper, Navigator>;
    // propagate neutral particles with just straight lines
    using NeutralStepper = Acts::StraightLineStepper;
    using NeutralPropagator = Acts::Propagator<NeutralStepper, Navigator>;
    // charged simulation with EM interactions
    using ChargedSelector = ActsFatras::ChargedSelector;
    using ChargedInteractions = ActsFatras::StandardChargedElectroMagneticInteractions;
    using ChargedSimulation = SingleParticleSimulation<ChargedPropagator, ChargedInteractions, 
                                                        HitSurfaceSelector,
                                                        ActsFatras::NoDecay>;
    // neutral simulation with photon conversion
    using NeutralSelector = ActsFatras::NeutralSelector;
    using NeutralInteractions = ActsFatras::InteractionList<ActsFatras::PhotonConversion>;
    using NeutralSimulation = SingleParticleSimulation<NeutralPropagator, NeutralInteractions, 
                                                        ActsFatras::NoSurface,
                                                        ActsFatras::NoDecay>;
    // full simulator type for charged and neutrals
    using Simulation = ActsFatras::MultiParticleSimulation<ChargedSelector, ChargedSimulation,
                                            NeutralSelector, NeutralSimulation>;

    // ==============================
    // Acts navigation / field
    // ==============================
    std::unique_ptr<Navigator> m_navigator;
    std::shared_ptr<ATLASMagneticFieldWrapper> m_bField;

    // ==============================
    // Top-level simulator
    // ==============================
    std::unique_ptr<Simulation> m_simulator;

    // ==============================
    // Methods
    // ==============================
    StatusCode configureSimulator();
    void debugTrackRegion(const G4FastTrack& fastTrack) const;
    std::vector<ActsFatras::Particle> buildActsInputFromG4(const G4Track& track);
    void handleSimulationFailures(const Acts::Result<ActsFatras::SingleParticleSimulationResult>& result);
    StatusCode applyPrimaryUpdate(const G4Track&, const std::vector<ActsFatras::Particle>& simulatedFinalParticles, G4FastStep& fastStep);
    StatusCode spawnSecondaries(const std::vector<ActsFatras::Particle>& simulatedFinal, const G4Track&, G4FastStep& fastStep);

    // Make hits cache, to be written to SG
    StatusCode createHitsFromG4(
        const EventContext& ctx,
        const G4Track& track,
        const Acts::TrackingGeometry& trackingGeometry,
        const std::vector<ActsFatras::Hit>& hits
    ) const;
    // inject fake (muons) particles
    StatusCode runDebugInjection(
      const EventContext& ctx,
      const G4Track& track,
      const Acts::GeometryContext& anygctx,
      const Acts::MagneticFieldContext& mctx,
      Generator& generator,
      G4FastStep& fastStep
    );

    // ==============================
    // Properties
    // ==============================
    // Random number service
    ServiceHandle<IAthRNGSvc> m_rngSvc{this, "RNGService", "AthRNGSvc"};
    ATHRNG::RNGWrapper* m_randomEngine ATLAS_THREAD_SAFE {};
    Gaudi::Property<std::string> m_randomEngineName{this, "RandomEngineName", "RandomEngineName", "Name of random number stream"};

    // Tracking geometry
    ServiceHandle<ActsTrk::ITrackingGeometrySvc> m_trackingGeometrySvc{this, "TrackingGeometrySvc", "ActsTrackingGeometrySvc"};
    std::shared_ptr<const Acts::TrackingGeometry> m_trackingGeometry;


    // Logging
    std::shared_ptr<const Acts::Logger> m_logger{nullptr};

    // ** Other Gaudi properties ** //    
    Gaudi::Property<double> m_interact_minPt{this, "Interact_MinPt", 50.0, "Min pT of the interactions (MeV)"};
    // Propagator options
    Gaudi::Property<bool>   m_meanEnergyLoss{this, "MeanEnergyLoss", true, "Toggle between mean and mode evaluation of energy loss"};
    Gaudi::Property<bool>   m_includeGradient{this, "IncludeGradient", true, "Boolean flag for inclusion of d(dEds)d(q/p) into energy loss"};
    Gaudi::Property<double> m_momentumCutOff{this, "MomentumCutOff", 0., "Cut-off value for the momentum in SI units"};
    // Propagator options
    Gaudi::Property<double> m_maxStep{this, "MaxSteps", 2000, "Max number of steps"};
    Gaudi::Property<double> m_maxRungeKuttaStepTrials{this, "MaxRungeKuttaStepTrials", 10000, "Max number of Runge-Kutta steps for the stepper step call"};
    Gaudi::Property<double> m_maxStepSize{this, "MaxStepSize", 3.0, "Max step size (converted to Acts::UnitConstants::m)"};
    Gaudi::Property<double> m_pathLimit{this, "PathLimit", 3000.0, "Track path limit (converted to Acts::UnitConstants::cm)"};
    Gaudi::Property<bool>   m_loopProtection{this, "LoopProtection", true, "Loop protection, it adapts the pathLimit"};
    Gaudi::Property<double> m_loopFraction{this, "LoopFraction", 0.5, "Allowed loop fraction, 1 is a full loop"};
    Gaudi::Property<double> m_tolerance{this, "Tolerance", 0.0001, "Tolerance for the error of the integration"};
    Gaudi::Property<double> m_stepSizeCutOff{this, "StepSizeCutOff", 0., "Cut-off value for the step size"};
    Gaudi::Property<std::map<int,int>> m_processTypeMap{this, "ProcessTypeMap", 
                                                            {{0,0}, {1,201}, {2,14}, {3,3}, {4,121}}, 
                                                            "proessType map <ActsFatras,G4>"
                                                    };
    //{{ActsFatras::GenerationProcess::eUndefined,0}, {ActsFatras::GenerationProcess::eDecay,201}, {ActsFatras::GenerationProcess::ePhotonConversion,14}, {ActsFatras::GenerationProcess::eBremsstrahlung,3}, {ActsFatras::GenerationProcess::eNuclearInteraction,121}}
    inline int getATLASProcessCode(ActsFatras::GenerationProcess  actspt){return m_processTypeMap[static_cast<uint32_t>(actspt)];};

    // ==============================
    // Hits writing
    // ==============================
    struct EventCache {
        std::vector<SiHit> pixelHits;
        std::vector<SiHit> sctHits;
    };
    mutable SG::SlotSpecificObj<EventCache> m_eventCache ATLAS_THREAD_SAFE; 
    EventCache& getCache(const EventContext& ctx) const;

    // set to false always for non-test
    Gaudi::Property<bool> m_debugInject{this, "DebugInjectParticle", false};

  protected:
    // ==============================
    // Hits cache
    // ==============================
    // For SiHit creation
    const PixelID*                       m_pixIdHelper{nullptr};             //!< the Pixel ID helper
    const SCT_ID*                        m_sctIdHelper{nullptr};             //!< the SCT ID helper

}; // end of ActsFatrasG4Tool class

#endif // G4ATLASTOOLS_ACTSFATRASG4TOOL_H



                                                     


