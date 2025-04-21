/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "src/ActsTrackStateOnSurfaceDecoratorAlg.h"
#include "StoreGate/WriteDecorHandle.h"
#include "StoreGate/ReadDecorHandle.h"
#include "TrkEventPrimitives/TrackStateDefs.h"

namespace ActsTrk {

  ActsTrackStateOnSurfaceDecoratorAlg::ActsTrackStateOnSurfaceDecoratorAlg(const std::string& name,
									   ISvcLocator *pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator)
  {}
  
  StatusCode ActsTrackStateOnSurfaceDecoratorAlg::initialize()
  {
    ATH_MSG_DEBUG("Initializing " << name() << " ...");

    ATH_CHECK(m_trackParticlesKey.initialize());
    m_decorator_actsTracks = m_trackParticlesKey.key() + "." + m_decorator_actsTracks.key();
    ATH_CHECK(m_decorator_actsTracks.initialize());
    m_trackMsosLink = m_trackParticlesKey.key() + "." + m_trackMsosLink.key();
    ATH_CHECK(m_trackMsosLink.initialize());
    
    ATH_CHECK(m_pixelMeasurementsKey.initialize());
    ATH_CHECK(m_stripMeasurementsKey.initialize());
    ATH_CHECK(m_pixelMsosKey.initialize());
    ATH_CHECK(m_stripMsosKey.initialize());
    
    return StatusCode::SUCCESS;
  }
  
  StatusCode ActsTrackStateOnSurfaceDecoratorAlg::execute(const EventContext& ctx) const
  {
    ATH_MSG_DEBUG("Executing " << name() << " ...");

    SG::ReadHandle<xAOD::TrackParticleContainer> trackParticleHandle = SG::makeHandle( m_trackParticlesKey, ctx );
    ATH_CHECK(trackParticleHandle.isValid());
    const xAOD::TrackParticleContainer* trackParticles = trackParticleHandle.cptr();

    SG::ReadHandle<xAOD::TrackMeasurementValidationContainer> pixelMeasurementHandle = SG::makeHandle( m_pixelMeasurementsKey, ctx );
    ATH_CHECK(pixelMeasurementHandle.isValid());
    const xAOD::TrackMeasurementValidationContainer* pixelMeasurements = pixelMeasurementHandle.cptr();
    
    SG::ReadHandle<xAOD::TrackMeasurementValidationContainer> stripMeasurementHandle = SG::makeHandle( m_stripMeasurementsKey, ctx );
    ATH_CHECK(stripMeasurementHandle.isValid());
    const xAOD::TrackMeasurementValidationContainer* stripMeasurements = stripMeasurementHandle.cptr();
    
    SG::WriteHandle<xAOD::TrackStateValidationContainer> pixelMsosHandle = SG::makeHandle( m_pixelMsosKey, ctx );
    ATH_CHECK( pixelMsosHandle.record(std::make_unique<xAOD::TrackStateValidationContainer>(),
				      std::make_unique<xAOD::TrackStateValidationAuxContainer>()) );
    xAOD::TrackStateValidationContainer* pixelMsos = pixelMsosHandle.ptr();
    
    SG::WriteHandle<xAOD::TrackStateValidationContainer> stripMsosHandle = SG::makeHandle( m_stripMsosKey, ctx );
    ATH_CHECK( stripMsosHandle.record(std::make_unique<xAOD::TrackStateValidationContainer>(),
				      std::make_unique<xAOD::TrackStateValidationAuxContainer>()) );
    xAOD::TrackStateValidationContainer* stripMsos = stripMsosHandle.ptr();

    // Decorators
    SG::ReadDecorHandle<xAOD::TrackParticleContainer, ElementLink<ActsTrk::TrackContainer>> decorator_trackLink(m_decorator_actsTracks, ctx);
    ATH_CHECK(decorator_trackLink.isValid());

    SG::WriteDecorHandle<xAOD::TrackParticleContainer,
			 std::vector< ElementLink< xAOD::TrackStateValidationContainer > > > decorator_msos_link( m_trackMsosLink, ctx );
    ATH_CHECK(decorator_msos_link.isValid());


    
    for (const xAOD::TrackParticle* trackParticle : *trackParticles) {
      ElementLink<ActsTrk::TrackContainer> trackLink = decorator_trackLink(*trackParticle);
      ATH_CHECK(trackLink.isValid());

      std::optional<ActsTrk::TrackContainer::ConstTrackProxy> optional_track = *trackLink;
      if ( not optional_track.has_value() ) {
	ATH_MSG_ERROR("Invalid track link for particle  " << trackParticle->index());
	return StatusCode::FAILURE;
      }
      ActsTrk::TrackContainer::ConstTrackProxy track = optional_track.value();

      std::vector< typename ActsTrk::TrackContainer::ConstTrackStateProxy > tsos {};
      tsos.reserve( track.nTrackStates() );


      // loop on track states
      track.container().trackStateContainer()
	.visitBackwards(track.tipIndex(),
			[&tsos]
			(const typename ActsTrk::TrackContainer::ConstTrackStateProxy& state)
			{
			  auto flags = state.typeFlags();
			  if (not flags.test(Acts::TrackStateFlag::MeasurementFlag) and
			      not flags.test(Acts::TrackStateFlag::OutlierFlag) and
			      not flags.test(Acts::TrackStateFlag::HoleFlag)) return;
			  tsos.push_back( state );
			});

      std::vector< ElementLink< xAOD::TrackStateValidationContainer > > msos {};
      msos.reserve( tsos.size() );

      for (const typename ActsTrk::TrackContainer::ConstTrackStateProxy& state : tsos) {
	const Acts::Surface& surface = state.referenceSurface();
	xAOD::UncalibMeasType detectorTypeFromId = getDetectorType( surface.geometryId().volume() );

	if ( detectorTypeFromId == xAOD::UncalibMeasType::PixelClusterType ) {
	  ATH_CHECK( storeTrackState(state,
				     *pixelMeasurements,
				     msos,
				     *pixelMsos) );
	  pixelMsos->back()->setDetType( Trk::TrackState::Pixel );  
	}
	else if ( detectorTypeFromId == xAOD::UncalibMeasType::StripClusterType ) {
	  ATH_CHECK( storeTrackState(state,
				     *stripMeasurements,
				     msos,
				     *stripMsos) );
	  stripMsos->back()->setDetType( Trk::TrackState::SCT );
	}
	else {
	  ATH_MSG_ERROR("Not recognized detector type");
	  return StatusCode::FAILURE;
	}
	
      } // loop on states

      decorator_msos_link(*trackParticle) = std::move(msos);
    } // loop on track particles

    return StatusCode::SUCCESS;
  }

  StatusCode ActsTrackStateOnSurfaceDecoratorAlg::storeTrackState(const typename ActsTrk::TrackContainer::ConstTrackStateProxy& state,
								  const xAOD::TrackMeasurementValidationContainer& measurements,
								  std::vector< ElementLink< xAOD::TrackStateValidationContainer > >& msosLinks,
								  xAOD::TrackStateValidationContainer& msosContainer) const
  {
    msosContainer.push_back( new xAOD::TrackStateValidation() );

    ElementLink< xAOD::TrackStateValidationContainer > elink( &msosContainer, msosContainer.back()->index() );
    ATH_CHECK( elink.isValid() );
    msosLinks.push_back( std::move(elink) );
    
    auto flags = state.typeFlags();
    if (not flags.test(Acts::TrackStateFlag::HoleFlag) ) {
      auto sl = state.getUncalibratedSourceLink().template get<ATLASUncalibSourceLink>();
      ATH_CHECK( sl != nullptr );
      const xAOD::UncalibratedMeasurement &cluster = getUncalibratedMeasurement(sl);    
      msosContainer.back()->setTrackMeasurementValidationLink( ElementLink<xAOD::TrackMeasurementValidationContainer>(&measurements, cluster.index()) );
    }
    
    if (flags.test(Acts::TrackStateFlag::HoleFlag)) {
      msosContainer.back()->setType( Trk::TrackStateOnSurface::Hole );
    } else if (flags.test(Acts::TrackStateFlag::OutlierFlag)) {
      msosContainer.back()->setType( Trk::TrackStateOnSurface::Outlier );
    } else {
      msosContainer.back()->setType( Trk::TrackStateOnSurface::Measurement );
    }
    
    return StatusCode::SUCCESS;
  }
  
  xAOD::UncalibMeasType ActsTrackStateOnSurfaceDecoratorAlg::getDetectorType(std::uint64_t volumeId) const
  {
    switch (volumeId) {
    case 2:
    case 25:
      return xAOD::UncalibMeasType::HGTDClusterType;
    case 22:
    case 23:
    case 24:
      return xAOD::UncalibMeasType::StripClusterType;
    case 8:
    case 9:
    case 10:
    case 13:
    case 14:
    case 15:
    case 16:
    case 18:
    case 19:
    case 20:
      return xAOD::UncalibMeasType::PixelClusterType;
    default:
	throw std::runtime_error("Cannot recognize volume id");
    };
  }
  
}

