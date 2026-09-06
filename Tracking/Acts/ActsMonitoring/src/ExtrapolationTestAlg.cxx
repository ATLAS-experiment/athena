/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ExtrapolationTestAlg.h"

// ATHENA


// ACTS
#include "Acts/Propagator/MaterialInteractor.hpp"
#include "Acts/Propagator/detail/SteppingLogger.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/Utilities/Helpers.hpp"
#include "Acts/Definitions/Units.hpp"
#include "Acts/Utilities/Logger.hpp"

// PACKAGE
#include "ActsGeometryInterfaces/GeometryContext.h"
#include "ActsGeometryInterfaces/ITrackingGeometrySvc.h"
#include "ActsInterop/Logger.h"

// OTHER
#include "CLHEP/Random/RandomEngine.h"

// STL
#include <fstream>
#include <string>

using namespace Acts::UnitLiterals;

namespace ActsTrk{
StatusCode ExtrapolationTestAlg::initialize() {

  ATH_MSG_DEBUG(name() << "::" << __FUNCTION__);

  ATH_CHECK(m_rndmGenSvc.retrieve());
  ATH_CHECK(m_extrapolationTool.retrieve());
  ATH_CHECK(m_trackingGeometrySvc.retrieve());
  ATH_CHECK( m_materialTrackCollectionKey.initialize() );
  ATH_CHECK(m_tree.init(this));
  return StatusCode::SUCCESS;
}

StatusCode ExtrapolationTestAlg::finalize() {
    ATH_CHECK(m_tree.write());
    return StatusCode::SUCCESS;
}
StatusCode ExtrapolationTestAlg::execute(const EventContext &ctx) {

  ATH_MSG_VERBOSE(name() << "::" << __FUNCTION__);

  ATHRNG::RNGWrapper *rngWrapper = m_rndmGenSvc->getEngine(this);
  rngWrapper->setSeed(name(), ctx);
  CLHEP::HepRandomEngine *rngEngine = rngWrapper->getEngine(ctx);

  ATH_MSG_VERBOSE("Extrapolating " << m_nParticlePerEvent << " particles");

  // Write to the collection to the EventStore
  SG::WriteHandle materialTracks{m_materialTrackCollectionKey, ctx};

  // Record the collection once per event if not already there
  if (!materialTracks.isPresent()) {
      ATH_CHECK(materialTracks.record(std::make_unique<ActsTrk::RecordedMaterialTrackCollection>()));
  }

  // Add the track to the recorded collection
  auto* coll = materialTracks.ptr();

  for (size_t i = 0; i < m_nParticlePerEvent; ++i) {
    double d0 = 0;
    double z0 = 0;
    double phi = rngEngine->flat() * 2 * std::numbers::pi - std::numbers::pi;
    std::vector<double> etaRange = m_etaRange;
    double etaMin = etaRange.at(0);
    double etaMax = etaRange.at(1);
    double eta = rngEngine->flat() * std::abs(etaMax - etaMin) + etaMin;

    std::vector<double> ptRange = m_ptRange;
    double ptMin = ptRange.at(0) * 1_GeV;
    double ptMax = ptRange.at(1) * 1_GeV;

    double pt = rngEngine->flat() * std::abs(ptMax - ptMin) + ptMin;

    Acts::Vector3 momentum(pt * std::cos(phi), pt * std::sin(phi),
                            pt * std::sinh(eta));

    double theta = momentum.theta();

    double charge = rngEngine->flat() > 0.5 ? -1 : 1;

    double qop = charge / momentum.norm();

    auto surface = Acts::Surface::makeShared<Acts::PerigeeSurface>(Amg::Vector3D::Zero());

    double t = 0;
    ATH_MSG_VERBOSE("Pseudo-particle: eta: " << eta << " phi: " << phi);

    Acts::BoundVector pars;
    // cppcheck-suppress constStatement; will be able to initialize this directly with eigen 3.4
    pars << d0, z0, phi, theta, qop, t;
    std::optional<Acts::BoundMatrix> cov = std::nullopt;

    if (charge != 0.) {
      // Perigee, no alignment -> default geo context
      Acts::BoundTrackParameters startParameters(std::move(surface), std::move(pars), std::move(cov), Acts::ParticleHypothesis::pion());
      auto result = m_extrapolationTool->propagationSteps(ctx, startParameters);
      if (!result.ok()) {
        ATH_MSG_WARNING("Extrapolation tool failed to extrapolate the track: "
                        << result.error().message());
        continue;
      }
      auto &output = result.value();
      if(output.first.size() == 0) {
        ATH_MSG_WARNING("Got ZERO steps from the extrapolation tool");
      }
      if (m_writePropStep) {
         ATH_CHECK(writePropagationSteps(ctx, output.first));
      }

      if(m_writeMaterialTracks && coll){
        Acts::RecordedMaterialTrack track;
        track.first.first = Acts::Vector3::Zero();
        track.first.second = momentum;
        track.second = std::move(output.second);
        coll->push_back(std::move(track));
      }
    }

    ATH_MSG_VERBOSE(name() << " execute done");
  }

  return StatusCode::SUCCESS;
}

StatusCode ExtrapolationTestAlg::writePropagationSteps(const EventContext& ctx, const StepVector& steps) {
      m_eventNum = ctx.eventID().event_number();
      for(const auto& step : steps) {
      Acts::GeometryIdentifier::Value volumeID    = 0;
      Acts::GeometryIdentifier::Value boundaryID  = 0;
      Acts::GeometryIdentifier::Value layerID     = 0;
      Acts::GeometryIdentifier::Value approachID  = 0;
      Acts::GeometryIdentifier::Value sensitiveID = 0;
      // get the identification from the surface first
      if (step.surface) {
        auto geoID  = step.surface->geometryId();
        sensitiveID = geoID.sensitive();
        approachID  = geoID.approach();
        layerID     = geoID.layer();
        boundaryID  = geoID.boundary();
        volumeID    = geoID.volume();
      }
      // a current volume overwrites the surface tagged one
      if (step.geoID != Acts::GeometryIdentifier()) {
        volumeID = step.geoID.volume();
      }
      // now fill
      m_s_sensitiveID.push_back(sensitiveID);
      m_s_approachID.push_back(approachID);
      m_s_layerID.push_back(layerID);
      m_s_boundaryID.push_back(boundaryID);
      m_s_volumeID.push_back(volumeID);

      m_s_pX.push_back(step.position.x());
      m_s_pY.push_back(step.position.y());
      m_s_pZ.push_back(step.position.z());
      m_s_pR.push_back(Acts::VectorHelpers::perp(step.position));
  }

  

    return m_tree.fill(ctx) ? StatusCode::SUCCESS : StatusCode::FAILURE;
}
  
}