/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "ActsGeometry/ActsExtrapolationTool.h"

// ATHENA
#include "GaudiKernel/IInterface.h"

// PACKAGE
#include "ActsGeometryInterfaces/ActsGeometryContext.h"
#include "ActsGeometry/ActsTrackingGeometrySvc.h"
#include "ActsGeometry/ActsTrackingGeometryTool.h"
#include "ActsInterop/Logger.h"

// ACTS
#include "Acts/Propagator/Navigator.hpp"
#include "Acts/Propagator/EigenStepper.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Propagator/ActorList.hpp"
#include <Acts/Propagator/StraightLineStepper.hpp>
#include "Acts/Propagator/EigenStepperDefaultExtension.hpp"

#include "Acts/Utilities/Logger.hpp"

// BOOST
#include <boost/variant/variant.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/static_visitor.hpp>

// STL
#include <iostream>
#include <memory>

namespace ActsExtrapolationDetail {

  
  using VariantPropagatorBase = boost::variant<
      Acts::Propagator<Acts::EigenStepper<Acts::EigenStepperDefaultExtension>, Acts::Navigator>>;

  class VariantPropagator : public VariantPropagatorBase
  {
  public:
    using VariantPropagatorBase::VariantPropagatorBase;
  };

}

using ActsExtrapolationDetail::VariantPropagator;


ActsExtrapolationTool::ActsExtrapolationTool(const std::string& type, const std::string& name,
    const IInterface* parent)
  : base_class(type, name, parent)
{
}


ActsExtrapolationTool::~ActsExtrapolationTool()
{
}


StatusCode
ActsExtrapolationTool::initialize()
{
  using namespace std::literals::string_literals;
  m_logger = makeActsAthenaLogger(this, std::string("ActsExtrapTool"), std::string("Prop"));

  ATH_MSG_INFO("Initializing ACTS extrapolation");

  m_logger = makeActsAthenaLogger(this, std::string("Prop"), std::string("ActsExtrapTool"));

  ATH_CHECK( m_trackingGeometryTool.retrieve() );
  std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry
    = m_trackingGeometryTool->trackingGeometry();

  Acts::Navigator navigator( Acts::Navigator::Config{ trackingGeometry } );

  if (m_fieldMode == "ATLAS"s) {    
    ATH_MSG_INFO("Using ATLAS magnetic field service");
    using Stepper = Acts::EigenStepper<Acts::EigenStepperDefaultExtension>;

    ATH_CHECK( m_fieldCacheCondObjInputKey.initialize() );
    auto bField = std::make_shared<ATLASMagneticFieldWrapper>();

    auto stepper = Stepper(std::move(bField));
    auto propagator = Acts::Propagator<Stepper, Acts::Navigator>(std::move(stepper),
                                                                 std::move(navigator),
								 logger().cloneWithSuffix("Prop"));
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

    ATH_MSG_INFO("Using constant magnetic field: (Bx, By, Bz) = (" << m_constantFieldVector[0] << ", " 
                                                                   << m_constantFieldVector[1] << ", " 
                                                                   << m_constantFieldVector[2] << ")");
    
    using Stepper = Acts::EigenStepper<Acts::EigenStepperDefaultExtension>;

    auto bField = std::make_shared<Acts::ConstantBField>(constantFieldVector);
    auto stepper = Stepper(std::move(bField));
    auto propagator = Acts::Propagator<Stepper, Acts::Navigator>(std::move(stepper),
                                                                 std::move(navigator));
    m_varProp = std::make_unique<VariantPropagator>(propagator);
  }

  ATH_MSG_INFO("ACTS extrapolation successfully initialized");
  return StatusCode::SUCCESS;
}


ActsPropagationOutput
ActsExtrapolationTool::propagationSteps(const EventContext& ctx,
                                        const Acts::BoundTrackParameters& startParameters,
                                        Acts::Direction navDir /*= Acts::Direction::Forward*/,
                                        double pathLimit /*= std::numeric_limits<double>::max()*/) const
{

  ATH_MSG_VERBOSE(name() << "::" << __FUNCTION__ << " begin");

  Acts::MagneticFieldContext mctx = getMagneticFieldContext(ctx);
  const ActsGeometryContext& geo_ctx
    = m_trackingGeometryTool->getGeometryContext(ctx);
  auto anygctx = geo_ctx.context();

  ActsPropagationOutput output;

  auto res = boost::apply_visitor([&](const auto& propagator) -> ResultType {
      using Propagator = std::decay_t<decltype(propagator)>;

      // Action list and abort list
      using ActorList =
      Acts::ActorList<SteppingLogger, Acts::MaterialInteractor, EndOfWorld>;
      using Options = typename Propagator::template Options<ActorList>;

       Options options = prepareOptions<Options>(anygctx, mctx, startParameters, navDir, pathLimit);

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
    ATH_MSG_ERROR("Got error during propagation: "
		              << res.error() << " " << res.error().message()
                  << ". Returning empty step vector.");
    return {};
  }
  output = std::move(*res);

  ATH_MSG_VERBOSE("Collected " << output.first.size() << " steps");
  if(output.first.size() == 0) {
    ATH_MSG_WARNING("ZERO steps returned by stepper, that is not typically a good sign");
  }

  ATH_MSG_VERBOSE(name() << "::" << __FUNCTION__ << " end");

  return output;
}



std::optional<const Acts::CurvilinearTrackParameters>
ActsExtrapolationTool::propagate(const EventContext& ctx,
                                 const Acts::BoundTrackParameters& startParameters,
                                 Acts::Direction navDir /*= Acts::Direction::Forward*/,
                                 double pathLimit /*= std::numeric_limits<double>::max()*/) const
{
  ATH_MSG_VERBOSE(name() << "::" << __FUNCTION__ << " begin");

  Acts::MagneticFieldContext mctx = getMagneticFieldContext(ctx);
  const ActsGeometryContext& geo_ctx
    = m_trackingGeometryTool->getGeometryContext(ctx);
  auto anygctx = geo_ctx.context();

  auto parameters = boost::apply_visitor([&](const auto& propagator) -> std::optional<const Acts::CurvilinearTrackParameters> {
      using Propagator = std::decay_t<decltype(propagator)>;

      // Action list and abort list
      using ActorList =
      Acts::ActorList<Acts::MaterialInteractor, EndOfWorld>;
      using Options = typename Propagator::template Options<ActorList>;

      Options options = prepareOptions<Options>(anygctx, mctx, startParameters, navDir, pathLimit);

      
      auto result = propagator.propagate(startParameters, options);
      if (!result.ok()) {
        ATH_MSG_ERROR("Got error during propagation:" << result.error()
                      << ". Returning empty parameters.");
        return std::nullopt;
      }
      return result.value().endParameters;
    }, *m_varProp);

  return parameters;
}

ActsPropagationOutput
ActsExtrapolationTool::propagationSteps(const EventContext& ctx,
                                        const Acts::BoundTrackParameters& startParameters,
                                        const Acts::Surface& target,
                                        Acts::Direction navDir /*= Acts::Direction::Forward*/,
                                        double pathLimit /*= std::numeric_limits<double>::max()*/) const
{
  ATH_MSG_VERBOSE(name() << "::" << __FUNCTION__ << " begin");

  ActsPropagationOutput output;

  Acts::MagneticFieldContext mctx = getMagneticFieldContext(ctx);
  const ActsGeometryContext& geo_ctx
    = m_trackingGeometryTool->getGeometryContext(ctx);
  auto anygctx = geo_ctx.context();

  auto res = boost::apply_visitor([&](const auto& propagator) -> ResultType {
      using Propagator = std::decay_t<decltype(propagator)>;

      // Action list and abort list
      using ActorList =
      Acts::ActorList<SteppingLogger, Acts::MaterialInteractor>;
      using Options = typename Propagator::template Options<ActorList>;

      Options options = prepareOptions<Options>(anygctx, mctx, startParameters, navDir, pathLimit);
      auto result = target.type() == Acts::Surface::Perigee  ?
        propagator.template propagate<Acts::BoundTrackParameters, Options,
                                    Acts::ForcedSurfaceReached, Acts::PathLimitReached>(startParameters, target, options) :
        propagator.template propagate<Acts::BoundTrackParameters, Options,
                                    Acts::SurfaceReached, Acts::PathLimitReached>(startParameters, target, options);

      
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
    ATH_MSG_ERROR("Got error during propagation:" << res.error()
                  << ". Returning empty step vector.");
    return {};
  }
  output = std::move(*res);

  ATH_MSG_VERBOSE("Collected " << output.first.size() << " steps");
  ATH_MSG_VERBOSE(name() << "::" << __FUNCTION__ << " end");

  return output;
}

std::optional<const Acts::BoundTrackParameters>
ActsExtrapolationTool::propagate(const EventContext& ctx,
                                 const Acts::BoundTrackParameters& startParameters,
                                 const Acts::Surface& target,
                                 Acts::Direction navDir /*= Acts::Direction::Forward*/,
                                 double pathLimit /*= std::numeric_limits<double>::max()*/) const
{
  
  ATH_MSG_VERBOSE(name() << "::" << __FUNCTION__ << " begin");
  
  Acts::MagneticFieldContext mctx = getMagneticFieldContext(ctx);
  const ActsGeometryContext& geo_ctx
    = m_trackingGeometryTool->getGeometryContext(ctx);
  auto anygctx = geo_ctx.context();

  auto parameters = boost::apply_visitor([&](const auto& propagator) -> std::optional<const Acts::BoundTrackParameters> {
      using Propagator = std::decay_t<decltype(propagator)>;

      // Action list and abort list
      using ActorList =
      Acts::ActorList<Acts::MaterialInteractor>;
      using Options = typename Propagator::template Options<ActorList>;

      Options options = prepareOptions<Options>(anygctx, mctx, startParameters, navDir, pathLimit);
      auto result = target.type() == Acts::Surface::Perigee  ?
        propagator.template propagate<Acts::BoundTrackParameters, Options,
                                    Acts::ForcedSurfaceReached, Acts::PathLimitReached>(startParameters, target, options) :
        propagator.template propagate<Acts::BoundTrackParameters, Options,
                                    Acts::SurfaceReached, Acts::PathLimitReached>(startParameters, target, options);
      if (!result.ok()) {
        ATH_MSG_ERROR("Got error during propagation: " << result.error()
                      << ". Returning empty parameters.");
        return std::nullopt;
      }
      return result.value().endParameters;
    }, *m_varProp);

  return parameters;
}

Acts::MagneticFieldContext ActsExtrapolationTool::getMagneticFieldContext(const EventContext& ctx) const {
  SG::ReadCondHandle<AtlasFieldCacheCondObj> readHandle{m_fieldCacheCondObjInputKey, ctx};
  if (!readHandle.isValid()) {
     std::stringstream msg;
     msg << "Failed to retrieve magnetic field condition data " << m_fieldCacheCondObjInputKey.key() << ".";
     throw std::runtime_error(msg.str());
  }
  const AtlasFieldCacheCondObj* fieldCondObj{*readHandle};

  return Acts::MagneticFieldContext(fieldCondObj);
}

template<typename OptionsType>
OptionsType ActsExtrapolationTool::prepareOptions(const Acts::GeometryContext& gctx,
                                                  const Acts::MagneticFieldContext& mctx,
                                                  const Acts::BoundTrackParameters& startParameters,
                                                  Acts::Direction navDir, 
                                                  double pathLimit) const { 
  using namespace Acts::UnitLiterals;
  OptionsType options(gctx, mctx);

  options.pathLimit = pathLimit;
  options.loopProtection
    = (Acts::VectorHelpers::perp(startParameters.momentum())
      < m_ptLoopers * 1_MeV);
  options.maxSteps = m_maxStep;
  options.direction = navDir;
  options.stepping.maxStepSize = m_maxStepSize * 1_m;

  auto& mInteractor = options.actorList.template get<Acts::MaterialInteractor>();
  mInteractor.multipleScattering = m_interactionMultiScatering;
  mInteractor.energyLoss = m_interactionEloss;
  mInteractor.recordInteractions = m_interactionRecord;

  return options;
}
