/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_ACTSTOOLS_ACTSFATRASSIMTOOL_H
#define ISF_ACTSTOOLS_ACTSFATRASSIMTOOL_H

// Gaudi
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"

// Athena
#include "AthenaKernel/IAthRNGSvc.h"
#include "AthenaKernel/RNGWrapper.h"
#include "CxxUtils/checker_macros.h"

// ISF
#include "ISF_Event/ISFParticle.h"
#include "ISF_Interfaces/BaseSimulatorTool.h"
#include "ISF_Interfaces/IParticleFilter.h"
#include "ISF_Interfaces/ITruthSvc.h"
#include "ActsFatrasWriteHandler.h"

// ACTS
#include "Acts/Utilities/UnitVectors.hpp"
#include "ActsInterop/Logger.h"
#include "ActsInterop/UnitConverters.h"
#include "Acts/Geometry/GeometryContext.hpp"
#include "Acts/MagneticField/MagneticFieldContext.hpp"
#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/Propagator/Navigator.hpp"
#include "Acts/Propagator/EigenStepper.hpp"
#include "Acts/Propagator/EigenStepperDefaultExtension.hpp"
#include "Acts/Propagator/StraightLineStepper.hpp"
#include "Acts/Propagator/detail/SteppingLogger.hpp"
#include "Acts/Propagator/ActorList.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Definitions/ParticleData.hpp"
#include "ActsFatras/EventData/ProcessType.hpp"
#include "ActsFatras/Kernel/InteractionList.hpp"
#include "ActsFatras/Kernel/Simulation.hpp"
#include "ActsFatras/Kernel/SimulationResult.hpp"
#include "ActsFatras/Physics/Decay/NoDecay.hpp"
#include "ActsFatras/Physics/StandardInteractions.hpp"
#include "ActsFatras/Physics/ElectroMagnetic/PhotonConversion.hpp"
#include "ActsFatras/Selectors/SurfaceSelectors.hpp"
// Tracking
#include "ActsGeometryInterfaces/ITrackingGeometryTool.h"
#include "ActsGeometry/ATLASMagneticFieldWrapper.h"

#include <algorithm>
#include <cassert>
#include <vector>

class AtlasDetectorID;

namespace iFatras {
  class ISimHitCreator;
}

namespace ISF {
class IParticleHelper;

/**
   @class ACTSFATRASSIMTOOL

   Standard ATLAS hit creator, with Acts-fatras simulation
*/

class ActsFatrasSimTool : public BaseSimulatorTool {
 
 public:
  /// Simple struct to select surfaces where hits should be generated.
  struct HitSurfaceSelector {
    /// Check if the surface should be used.
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
  template <typename propagator_t, typename interactions_t,
            typename hit_surface_selector_t, typename decay_t>
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
    SingleParticleSimulation(propagator_t &&propagator_,
                            std::shared_ptr<const Acts::Logger> localLogger_)
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
    Acts::Result<ActsFatras::SimulationResult> simulate(
        const Acts::GeometryContext &geoCtx,
        const Acts::MagneticFieldContext &magCtx, generator_t &generator,
        const ActsFatras::Particle &particle) const {
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
      Acts::BoundTrackParameters startPoint = Acts::BoundTrackParameters::createCurvilinear(
          particle.fourPosition(), particle.direction(),
          particle.qOverP(), std::nullopt, particle.hypothesis());
      options.pathLimit = pathLimit * Acts::UnitConstants::cm;
      options.loopProtection = loopProtection;
      options.maxSteps = maxStep;
      options.stepping.maxStepSize = maxStepSize * Acts::UnitConstants::m;

      auto result = propagator.propagate(startPoint, options);
      if (not result.ok()) {
        return result.error();
      }
      return result.value().template get<Result>();
    }
  };// end of SingleParticleSimulation

  // Standard generator
  using Generator = std::ranlux48;
  // Use default navigator
  using Navigator = Acts::Navigator;
  // Propagate charged particles numerically in the B-field
  using ChargedStepper = Acts::EigenStepper<Acts::EigenStepperDefaultExtension>;
  using ChargedPropagator = Acts::Propagator<ChargedStepper, Navigator>;
  // Propagate neutral particles in straight lines
  using NeutralStepper = Acts::StraightLineStepper;
  using NeutralPropagator = Acts::Propagator<NeutralStepper, Navigator>;

  // ===============================
  // Setup ActsFatras simulator types
  // Charged
  using ChargedSelector = ActsFatras::ChargedSelector;
  using ChargedInteractions =
          ActsFatras::StandardChargedElectroMagneticInteractions;
  using ChargedSimulation = SingleParticleSimulation<
          ChargedPropagator, ChargedInteractions, HitSurfaceSelector,
          ActsFatras::NoDecay>;
  // Neutral
  using NeutralSelector = ActsFatras::NeutralSelector;
  using NeutralInteractions = ActsFatras::InteractionList<ActsFatras::PhotonConversion>;
  using NeutralSimulation = SingleParticleSimulation<
          NeutralPropagator, NeutralInteractions, ActsFatras::NoSurface,
          ActsFatras::NoDecay>;
  // Combined
  using Simulation = ActsFatras::Simulation<ChargedSelector, ChargedSimulation,
                                            NeutralSelector, NeutralSimulation>;
  // ===============================

  ActsFatrasSimTool(const std::string& type, const std::string& name,
                    const IInterface* parent);
  virtual ~ActsFatrasSimTool();

  // ISF BaseSimulatorTool Interface methods
  virtual StatusCode initialize() override;
  virtual StatusCode simulate(const EventContext& ctx, ISFParticle& isp, ISFParticleContainer&,
                              McEventCollection*) override;
  virtual StatusCode simulateVector(
            const EventContext& ctx,
            const ISFParticleVector& particles,
            ISFParticleContainer& secondaries,
            McEventCollection* mcEventCollection, McEventCollection *shadowTruth=nullptr) override;
  virtual StatusCode setupEvent(const EventContext&) override {
    ATH_CHECK(m_truthRecordSvc->initializeTruthCollection());
    m_pixelSiHits.Clear();
    m_sctSiHits.Clear();
    return StatusCode::SUCCESS; };
  virtual StatusCode releaseEvent(const EventContext& ctx) override {
    std::vector<SiHitCollection> hitcolls;
    hitcolls.push_back(m_pixelSiHits);
    hitcolls.push_back(m_sctSiHits);
    ATH_CHECK(m_ActsFatrasWriteHandler->WriteHits(hitcolls,ctx));
    ATH_CHECK(m_truthRecordSvc->releaseEvent());
    return StatusCode::SUCCESS; };
  virtual ISF::SimulationFlavor simFlavor() const override{
    return ISF::Fatras; };

  virtual Acts::MagneticFieldContext getMagneticFieldContext(
    const EventContext&) const;

 private:
  // For sihit creation
  SiHitCollection m_pixelSiHits;
  SiHitCollection m_sctSiHits;
  // Templated tool retrieval
  template <class T>
  StatusCode retrieveTool(ToolHandle<T>& thandle) {
    if (!thandle.empty() && thandle.retrieve().isFailure()) {
      ATH_MSG_FATAL("Cannot retrieve " << thandle << ". Abort.");
      return StatusCode::FAILURE;
    } else ATH_MSG_DEBUG("Successfully retrieved " << thandle);
    return StatusCode::SUCCESS;
  }

  // Random number service
  ServiceHandle<IAthRNGSvc> m_rngSvc{this, "RNGService", "AthRNGSvc"};
  ATHRNG::RNGWrapper* m_randomEngine ATLAS_THREAD_SAFE {};
  Gaudi::Property<std::string> m_randomEngineName{this, "RandomEngineName",
    "RandomEngineName", "Name of random number stream"};

  // Tracking geometry
  PublicToolHandle<ActsTrk::ITrackingGeometryTool> m_trackingGeometryTool{
      this, "TrackingGeometryTool", "ActsTrackingGeometryTool"};
  std::shared_ptr<const Acts::TrackingGeometry> m_trackingGeometry;

  // Magnetic field
  SG::ReadCondHandleKey<AtlasFieldCacheCondObj> m_fieldCacheCondObjInputKey {this, "AtlasFieldCacheCondObj", "fieldCondObj", "Name of the Magnetic Field conditions object key"};

  // Logging
  std::shared_ptr<const Acts::Logger> m_logger{nullptr};

  // ISF Tools
  PublicToolHandle<ISF::IParticleFilter> m_particleFilter{
      this, "ParticleFilter", "", "Particle filter kinematic cuts, etc."};

  ServiceHandle<ISF::ITruthSvc> m_truthRecordSvc{this, "TruthRecordService", "ISF_TruthRecordSvc", ""};

  // ActsFatrasHitConvtTool
  ToolHandle<ActsFatrasWriteHandler> m_ActsFatrasWriteHandler{
      this, "ActsFatrasWriteHandler", "ActsFatrasWriteHandler"};

  Gaudi::Property<double> m_interact_minPt{this, "Interact_MinPt", 50.0,
      "Min pT of the interactions (MeV)"};

  // DensEnviroment Propergator option
  Gaudi::Property<bool> m_meanEnergyLoss{this, "MeanEnergyLoss", true, "Toggle between mean and mode evaluation of energy loss"};
  Gaudi::Property<bool> m_includeGgradient{this, "IncludeGgradient", true, "Boolean flag for inclusion of d(dEds)d(q/p) into energy loss"};
  Gaudi::Property<double> m_momentumCutOff{this, "MomentumCutOff", 0., "Cut-off value for the momentum in SI units"};
  // Propergator option
  Gaudi::Property<double> m_maxStep{this, "MaxSteps", 1000,
       "Max number of steps"};
  Gaudi::Property<double> m_maxRungeKuttaStepTrials{this, "MaxRungeKuttaStepTrials", 10000,
       "Maximum number of Runge-Kutta steps for the stepper step call"};
  Gaudi::Property<double> m_maxStepSize{this, "MaxStepSize", 3.0,
      "Max step size (converted to Acts::UnitConstants::m)"};
  Gaudi::Property<double> m_pathLimit{this, "PathLimit", 100.0,
      "Track path limit (converted to Acts::UnitConstants::cm)"};
  Gaudi::Property<bool> m_loopProtection{this, "LoopProtection", true,
      "Loop protection, it adapts the pathLimit"};
  Gaudi::Property<double> m_loopFraction{this, "LoopFraction", 0.5,
      "Allowed loop fraction, 1 is a full loop"};
  Gaudi::Property<double> m_tolerance{this, "Tolerance", 0.0001,
       "Tolerance for the error of the integration"};
  Gaudi::Property<double> m_stepSizeCutOff{this, "StepSizeCutOff", 0.,
       "Cut-off value for the step size"};

  Gaudi::Property<std::map<int,int>> m_processTypeMap{this, "ProcessTypeMap",
      {{0,0}, {1,201}, {2,14}, {3,3}, {4,121}}, "proessType map <ActsFatras,G4>"};
      //{{ActsProcessType::eUndefined,0}, {ActsProcessType::eDecay,201}, {ActsProcessType::ePhotonConversion,14}, {ActsProcessType::eBremsstrahlung,3}, {ActsProcessType::eNuclearInteraction,121}}
  inline int getATLASProcessCode(ActsFatras::ProcessType actspt){return m_processTypeMap[static_cast<uint32_t>(actspt)];};
};

}  // namespace ISF

#endif