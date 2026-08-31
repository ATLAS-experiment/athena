/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "ExtrapolationTool.h"

#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"

// PACKAGE
#include "ActsGeometryInterfaces/GeometryContext.h"
#include "ActsInterop/Logger.h"

// ACTS

#include "Acts/Geometry/VolumeBounds.hpp"
#include "Acts/Propagator/Navigator.hpp"
#include "Acts/Propagator/EigenStepper.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Propagator/ActorList.hpp"
#include <Acts/Propagator/StraightLineStepper.hpp>
#include "Acts/Propagator/EigenStepperDefaultExtension.hpp"

#include "Acts/Utilities/Logger.hpp"


// STL
#include <iostream>
#include <memory>

namespace {
    using SteppingLogger = Acts::detail::SteppingLogger;
    using EndOfWorld = Acts::EndOfWorldReached;

    using CurvedStepper_t = Acts::EigenStepper<Acts::EigenStepperDefaultExtension>;
    using CurvedPropagator_t = Acts::Propagator<CurvedStepper_t, Acts::Navigator>;
    using StraightStepper_t = Acts::StraightLineStepper;
    using StraightPropagator_t = Acts::Propagator<StraightStepper_t, Acts::Navigator>;

namespace {
    /** @brief Actor which aborts the propagation if the volume with
     *         the given geometry ID is entered */
    struct PassedVolumeAborter{
        /** @brief Geomertry ID of the volume to reach */
        Acts::GeometryIdentifier targetVolumeId{};
        using VolumeAbort = ActsTrk::IExtrapolationTool::VolumeAbort;
         /** @brief Flag to toggle when the volume is reached */      
        VolumeAbort stopVolumeFlag{VolumeAbort::atExit};

        /** @brief Default constructor */
        explicit PassedVolumeAborter() = default;
        /** @brief Implementation of the actor interface to check whether the
         *         propagation shall be aborted. True means abort. */
        template <typename propagator_state_t, typename stepper_t,
                    typename navigator_t>
        bool checkAbort(propagator_state_t& state, const stepper_t& stepper,
                        const navigator_t& navigator, const Acts::Logger& logger) const  {
            /** Just abort invalid propagations without configured Geometry ID */
            if (targetVolumeId == Acts::GeometryIdentifier{}) {
                ACTS_WARNING("PassedVolume aborter | Target volume not set");
                return true;
            }
            const Acts::TrackingVolume* currentVolume = navigator.currentVolume(state.navigation);
            if (currentVolume == nullptr) {
              return false;
            }
            
            const Acts::Surface* surface = navigator.currentSurface(state.navigation);
            /// Skip portal less navigation states
            if (surface == nullptr || surface->geometryId().boundary() == 0) {
                return false;
            }
            ACTS_VERBOSE("PassedVolume - Investigate portal surface "<<surface->geometryId());
            if (surface->geometryId().withBoundary(0) == targetVolumeId) {
                ACTS_VERBOSE("PassedVolume - The outer boundary surface is crossed ");
                return true;
            }
            if (stopVolumeFlag == VolumeAbort::atExit) {
                return false;
            }
            /// Check the portals
            for (const Acts::Portal& portal : currentVolume->portals()) {
                if (portal.surface().geometryId() != surface->geometryId()) {
                    continue;
                }
                auto res = portal.resolveVolume(state.navigation.options.geoContext, 
                                                 stepper.position(state.stepping), 
                                                 stepper.direction(state.stepping));
                if (!res.ok()) {
                  ACTS_WARNING("Failed to resolve volume through portal: "
                                          << res.error().message());
                  return true;
                }
                return (*res)->geometryId() == targetVolumeId;
            }
            ACTS_WARNING("PassedVolume - Cannot find portal associated with "<<surface->geometryId());
            return true;
        }
    };

    /** @brief Actor which aborts if the surface has been surpassed by the
     *         propagator by an distance X */
    struct PassedSurfaceAborter {
        /** @brief Target surface that needs to be surpassed */
        const Acts::Surface* targetSurface = nullptr;
        /** @brief The distance that the propagation needs to be at least away */
        double surpassedDistance{0.};
        /** @brief Default constructor */
        explicit PassedSurfaceAborter() = default;
 
        /** @brief Implementation of the actor interface to check whether the
         *         propagation shall be aborted. True means abort. */
        template <typename propagator_state_t, typename stepper_t,
                    typename navigator_t>
        bool checkAbort(propagator_state_t& state, const stepper_t& stepper,
                        const navigator_t& /*navigator*/, const Acts::Logger& logger) const {
            if (targetSurface == nullptr) {
                ACTS_WARNING("PassedSurfaceAborter aborter | Target surface not set.");
                return true;
            }

            const Acts::MultiIntersection3D multiIntersection = targetSurface->intersect(state.geoContext, 
                                                                                   stepper.position(state.stepping),
                                                                                   state.options.direction * stepper.direction(state.stepping),
                                                                                   Acts::BoundaryTolerance::Infinite());
                                                                            
            const Acts::Intersection3D closestIntersection = multiIntersection.closest();
            ACTS_VERBOSE("PassedSurfaceAborter aborter | Propagation is "<<closestIntersection.pathLength()
                    <<" away from target surface "
                    <<targetSurface->toString(state.geoContext)<<". Abort if distance is "
                    <<std::copysign(surpassedDistance, -1.)<<".");
            return closestIntersection.pathLength() < std::copysign(surpassedDistance, -1.);
        }
    };
}


}

namespace ActsExtrapolationDetail {
  using VariantPropagatorBase = std::variant<CurvedPropagator_t, StraightPropagator_t>;

  class VariantPropagator : public VariantPropagatorBase
  {
  public:
    using VariantPropagatorBase::VariantPropagatorBase;
  };
}


using ActsExtrapolationDetail::VariantPropagator;

namespace ActsTrk{


ExtrapolationTool::ExtrapolationTool(const std::string& type, 
                                     const std::string& name,
                                     const IInterface* parent):
    base_class{type,name, parent} {}

ExtrapolationTool::~ExtrapolationTool() = default;

StatusCode
ExtrapolationTool::initialize()
{


  ATH_MSG_INFO("Initializing ACTS extrapolation");

  m_logger = makeActsAthenaLogger(this, name());

  ATH_CHECK( m_trackingGeometrySvc.retrieve() );

  Acts::Navigator::Config navConfig{m_trackingGeometrySvc->trackingGeometry()};
  Acts::Navigator navigator{std::move(navConfig), logger().clone()};
  
  ATH_CHECK(m_ctxProvider.initialize());
  if (m_fieldMode == "ATLAS") {    
    ATH_MSG_INFO("Using ATLAS magnetic field service");

    auto bField = std::make_shared<ATLASMagneticFieldWrapper>();

    CurvedStepper_t stepper{std::move(bField)};
    CurvedPropagator_t propagator{std::move(stepper), std::move(navigator),
                                  logger().clone()};
    m_varProp = std::make_unique<VariantPropagator>(propagator);
  }
  else if (m_fieldMode == "Constant") {
    if (m_constantFieldVector.value().size() != 3)
    {
      ATH_MSG_ERROR("Incorrect field vector size. Using empty field.");
      return StatusCode::FAILURE; 
    }
    
    Acts::Vector3 constantFieldVector = Acts::Vector3(m_constantFieldVector[0], 
                                                      m_constantFieldVector[1], 
                                                      m_constantFieldVector[2]);

    ATH_MSG_INFO("Using constant magnetic field: (Bx, By, Bz) = "
                <<Amg::toString(constantFieldVector));
 
    auto bField = std::make_shared<Acts::ConstantBField>(constantFieldVector);
    CurvedStepper_t stepper{std::move(bField)};
    CurvedPropagator_t propagator{std::move(stepper), std::move(navigator), logger().clone()};
    m_varProp = std::make_unique<VariantPropagator>(propagator);
  } else if (m_fieldMode == "StraightLine") {
      Acts::StraightLineStepper stepper{};
      StraightPropagator_t propagator{stepper, std::move(navigator), logger().clone()};
      m_varProp = std::make_unique<VariantPropagator>(propagator);
  } else {
     ATH_MSG_FATAL("Invalid mode provided "<<m_fieldMode<<". Allowed  : \"ATLAS\", \"Constant\", \"StraightLine\".");
     return StatusCode::FAILURE;
  }

  ATH_MSG_INFO("ACTS extrapolation successfully initialized");
  return StatusCode::SUCCESS;
}


Acts::Result<ExtrapolationTool::PropagationOutput>
ExtrapolationTool::propagationSteps(const EventContext& ctx,
                                    const Acts::BoundTrackParameters& startParameters,
                                    const Acts::Direction navDir,
                                    const double pathLimit) const {

  ATH_MSG_VERBOSE(name() << "::" << __FUNCTION__ << " begin");

  const Acts::MagneticFieldContext mfContext = m_ctxProvider.getMagneticFieldContext(ctx);
  const Acts::GeometryContext tgContext = m_ctxProvider.getGeometryContext(ctx);
  
  PropagationOutput output;

  auto res = std::visit([&](const auto& propagator) -> Acts::Result<ExtrapolationTool::PropagationOutput> {
      using Propagator = std::decay_t<decltype(propagator)>;

      // Action list and abort list
      using ActorList =
      Acts::ActorList<SteppingLogger, Acts::MaterialInteractor, EndOfWorld>;
      using Options = typename Propagator::template Options<ActorList>;

      Options options = prepareOptions<Options>(tgContext, mfContext, startParameters, navDir, pathLimit);

      auto result = propagator.propagate(startParameters, options);
      if (!result.ok()) {
        return result.error();
      }
      auto& propRes = *result;

      auto steppingResults = propRes.template get<SteppingLogger::result_type>();
      auto materialResult = propRes.template get<Acts::MaterialInteractor::result_type>();
      output.first = std::move(steppingResults.steps);
      output.second = std::move(materialResult);
      // try to force return value optimization, not sure this is necessary
      return std::move(output);
    }, *m_varProp);

  if (!res.ok()) {
    ATH_MSG_DEBUG("Got error during propagation: "
		              << res.error() << " " << res.error().message()
                  << ". Returning empty step vector.");
    return res.error();
  }
  output = std::move(*res);

  ATH_MSG_VERBOSE("Collected " << output.first.size() << " steps");
  if(output.first.size() == 0) {
    ATH_MSG_WARNING("ZERO steps returned by stepper, that is not typically a good sign");
  }

  ATH_MSG_VERBOSE(name() << "::" << __FUNCTION__ << " end");

  return output;
}



Acts::Result<Acts::BoundTrackParameters>
    ExtrapolationTool::propagate(const EventContext& ctx,
                                 const Acts::BoundTrackParameters& startParameters,
                                 const Acts::Direction navDir, 
                                 const double pathLimit) const
{
  ATH_MSG_VERBOSE(name() << "::" << __FUNCTION__ << " begin");

  const Acts::MagneticFieldContext mfContext = m_ctxProvider.getMagneticFieldContext(ctx);
  const Acts::GeometryContext tgContext = m_ctxProvider.getGeometryContext(ctx);

  auto parameters = std::visit([&](const auto& propagator) -> Acts::Result<Acts::BoundTrackParameters> {
      using Propagator = std::decay_t<decltype(propagator)>;

      // Action list and abort list
      using ActorList =
      Acts::ActorList<Acts::MaterialInteractor, EndOfWorld>;
      using Options = typename Propagator::template Options<ActorList>;

      Options options = prepareOptions<Options>(tgContext, mfContext, startParameters, navDir, pathLimit);

      
      auto result = propagator.propagate(startParameters, options);
      if (!result.ok()) {
        ATH_MSG_DEBUG("Got error during propagation:" << result.error());
        return result.error();
      }
      if (!result.value().endParameters.has_value()) {
        ATH_MSG_DEBUG("Propagation did not result in valid end parameters.");
        return Acts::PropagatorError::Failure;
      }
      return result.value().endParameters.value();
    }, *m_varProp);

  return parameters;
}

Acts::Result<ExtrapolationTool::PropagationOutput>
  ExtrapolationTool::propagationSteps(const EventContext& ctx,
                                      const Acts::BoundTrackParameters& startParameters,
                                      const Acts::Surface& target,
                                      const Acts::Direction navDir, 
                                      const double pathLimit) const {
  ATH_MSG_VERBOSE(name() << "::" << __FUNCTION__ << " begin");

  PropagationOutput output;

  const Acts::MagneticFieldContext mfContext = m_ctxProvider.getMagneticFieldContext(ctx);
  const Acts::GeometryContext tgContext = m_ctxProvider.getGeometryContext(ctx);

  auto res = std::visit([&](const auto& propagator) -> Acts::Result<PropagationOutput> {
      using Propagator = std::decay_t<decltype(propagator)>;

      // Action list and abort list
      using ActorList =
      Acts::ActorList<SteppingLogger, Acts::MaterialInteractor>;
      using Options = typename Propagator::template Options<ActorList>;

      Options options = prepareOptions<Options>(tgContext, mfContext, startParameters, navDir, pathLimit);
      auto result = target.type() == Acts::Surface::Perigee  ?
        propagator.template propagate<Options, Acts::ForcedSurfaceReached, Acts::PathLimitReached>(startParameters, target, options) :
        propagator.template propagate<Options, Acts::SurfaceReached, Acts::PathLimitReached>(startParameters, target, options);

      
      if (!result.ok()) {
        return result.error();
      }
      auto& propRes = *result;

      auto steppingResults = propRes.template get<SteppingLogger::result_type>();
      auto materialResult = propRes.template get<Acts::MaterialInteractor::result_type>();
      output.first = std::move(steppingResults.steps);
      output.second = std::move(materialResult);
      return std::move(output);
    }, *m_varProp);

  if (!res.ok()) {
    ATH_MSG_DEBUG("Got error during propagation:" << res.error()
                  << ". Returning empty step vector.");
    return res.error();
  }
  output = std::move(*res);

  ATH_MSG_VERBOSE("Collected " << output.first.size() << " steps");
  ATH_MSG_VERBOSE(name() << "::" << __FUNCTION__ << " end");

  return output;
}

Acts::Result<Acts::BoundTrackParameters>
    ExtrapolationTool::propagate(const EventContext& ctx,
                                 const Acts::BoundTrackParameters& startParameters,
                                 const Acts::Surface& target,
                                 const Acts::Direction navDir, const double pathLimit) const
{
  
  ATH_MSG_VERBOSE(name() << "::" << __FUNCTION__ << " begin");
  
  const Acts::MagneticFieldContext mfContext = m_ctxProvider.getMagneticFieldContext(ctx);
  const Acts::GeometryContext tgContext = m_ctxProvider.getGeometryContext(ctx);

  auto parameters = std::visit([&](const auto& propagator) -> Acts::Result<Acts::BoundTrackParameters> {
      using Propagator = std::decay_t<decltype(propagator)>;

      // Action list and abort list
      using ActorList =
      Acts::ActorList<Acts::MaterialInteractor>;
      using Options = typename Propagator::template Options<ActorList>;

      Options options = prepareOptions<Options>(tgContext, mfContext, startParameters, navDir, pathLimit);
      auto result = target.type() == Acts::Surface::Perigee  ?
        propagator.template propagate<Options, Acts::ForcedSurfaceReached, Acts::PathLimitReached>(startParameters, target, options) :
        propagator.template propagate<Options, Acts::SurfaceReached, Acts::PathLimitReached>(startParameters, target, options);
      if (!result.ok()) {
        ATH_MSG_DEBUG("Got error during propagation: " << result.error());
        return result.error();
      }
      if (!result.value().endParameters.has_value()) {
        ATH_MSG_DEBUG("Propagation did not result in valid end parameters.");
        return Acts::PropagatorError::Failure;
      }
      return result.value().endParameters.value();
    }, *m_varProp);

  return parameters;
}


template<typename OptionsType>
OptionsType ExtrapolationTool::prepareOptions(const Acts::GeometryContext& gctx,
                                              const Acts::MagneticFieldContext& mfContext,
                                              const Acts::BoundTrackParameters& startParameters,
                                              Acts::Direction navDir, double pathLimit) const { 
  using namespace Acts::UnitLiterals;
  OptionsType options(gctx, mfContext);

  options.pathLimit = pathLimit;
  options.loopProtection
    = (Acts::VectorHelpers::perp(startParameters.momentum())
      < m_ptLoopers * 1_MeV);
  options.maxSteps = m_maxStep;
  options.direction = navDir;
  options.stepping.maxStepSize = m_maxStepSize * 1_m;
  options.maxTargetSkipping = m_maxSurfSkip;
  options.surfaceTolerance = m_surfTolerance;
  auto& mInteractor = options.actorList.template get<Acts::MaterialInteractor>();
  mInteractor.multipleScattering = m_interactionMultiScatering;
  mInteractor.energyLoss = m_interactionEloss;
  mInteractor.recordInteractions = m_interactionRecord;
  return options;
}

Acts::Result<ExtrapolationTool::BoundParamVec_t> 
    ExtrapolationTool::propagateAndRecord(const EventContext& ctx,
                                          const Acts::BoundTrackParameters& startParameters,
                                          const SurfaceRecordOptions& recordOpts) const {

  const Acts::MagneticFieldContext mfContext = m_ctxProvider.getMagneticFieldContext(ctx);
  const Acts::GeometryContext tgContext = m_ctxProvider.getGeometryContext(ctx);

    return std::visit([&](const auto& propagator) -> Acts::Result<BoundParamVec_t> {
        using Propagator = std::decay_t<decltype(propagator)>;

        // Action list and abort list
        using ParamRecorder_t = Acts::BoundParameterRecorder<Acts::SurfaceSelector>;
        return std::visit([&](const auto target) -> Acts::Result<BoundParamVec_t> {
            using Target_t = std::decay_t<decltype(target)>;

            using TargetAborter_t = std::conditional_t<std::is_same_v<Target_t, const Acts::Surface*>,
                                                       PassedSurfaceAborter, PassedVolumeAborter>;
   
            using ActorList = Acts::ActorList<ParamRecorder_t, Acts::MaterialInteractor,  
                                              TargetAborter_t, EndOfWorld>;
   
            using Options = typename Propagator::template Options<ActorList>;

            auto propOptions = prepareOptions<Options>(tgContext, mfContext, startParameters, 
                                                        recordOpts.navDir, recordOpts.pathLimit);

            auto& surfaceRecorder = propOptions.actorList.template get<ParamRecorder_t>();
            surfaceRecorder.selector.selectSensitive = recordOpts.recordSensitive;
            surfaceRecorder.selector.selectMaterial = recordOpts.recordMaterial;
            surfaceRecorder.selector.selectPassive = recordOpts.recordPassive;

            auto& aborter = propOptions.actorList.template get<TargetAborter_t>();
            if constexpr(std::is_same_v<Target_t, const Acts::Surface*>) {
              aborter.targetSurface = target;
              aborter.surpassedDistance = recordOpts.extraPathLength;
            } else {
              /** Define the volume abort condition. If the propagation 
               *  leaves the volume or enters the volume */
              aborter.targetVolumeId = target->geometryId();
              aborter.stopVolumeFlag = recordOpts.stopVolumeFlag;
            }
            /** Execute the propagation */
            auto propResult = propagator.propagate(startParameters, propOptions);
            if (!propResult.ok()) {
                ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - Propagation failed.");
                return Acts::Result<BoundParamVec_t>::failure(std::make_error_code(std::errc::invalid_argument));
            }
            auto& result = *propResult;
            return Acts::Result<BoundParamVec_t>::success(std::move(result.template get<BoundParamVec_t>()));
        }, recordOpts.target);
    }, *m_varProp);
  }

  Acts::Result<Acts::BoundTrackParameters> ExtrapolationTool::propagate(const EventContext& ctx,
                                                                        const Acts::BoundTrackParameters& startParameters,
                                                                        const Acts::TrackingVolume& target,
                                                                        const VolumeAbort stopVolumeFlag,
                                                                        const Acts::Direction navDir,
                                                                        const double pathLimit) const {
    const Acts::MagneticFieldContext mfContext = m_ctxProvider.getMagneticFieldContext(ctx);
    const Acts::GeometryContext tgContext = m_ctxProvider.getGeometryContext(ctx);

    return std::visit([&](const auto& propagator) -> Acts::Result<Acts::BoundTrackParameters> {
      using Propagator = std::decay_t<decltype(propagator)>;
      using ActorList = Acts::ActorList<Acts::MaterialInteractor, PassedVolumeAborter, EndOfWorld>;

      using Options = typename Propagator::template Options<ActorList>;

      auto propOptions = prepareOptions<Options>(tgContext, mfContext, startParameters, 
                                                 navDir, pathLimit);

      auto& aborter = propOptions.actorList.template get<PassedVolumeAborter>();
      aborter.targetVolumeId = target.geometryId();
      aborter.stopVolumeFlag = stopVolumeFlag;
        
      /** Execute the propagation */
      auto propResult = propagator.propagate(startParameters, propOptions);
      if (!propResult.ok()) {
          ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - Propagation failed.");
          return Acts::PropagatorError::Failure;
      }
      if (!propResult.ok()) {
        ATH_MSG_DEBUG("Got error during propagation: " << propResult.error());
        return propResult.error();
      }
      if (!propResult.value().endParameters.has_value()) {
        ATH_MSG_DEBUG("Propagation did not result in valid end parameters.");
        return Acts::PropagatorError::Failure;
      }
      return propResult.value().endParameters.value();
    }, *m_varProp);
  }
}