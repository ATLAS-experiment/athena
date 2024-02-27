/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "src/SeedingAlg.h"

// ACTS
#include "Acts/Definitions/Units.hpp"
#include "Acts/MagneticField/MagneticFieldContext.hpp"
#include "ActsGeometry/ATLASMagneticFieldWrapper.h"
#include "Acts/Seeding/BinnedGroup.hpp"
#include "Acts/Seeding/SeedFilter.hpp"
#include "Acts/Seeding/SeedFinder.hpp"

#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "TrkSpacePoint/SpacePointCollection.h"
#include "SiSPSeededTrackFinderData/SiSpacePointForSeed.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"


#include "SiSPSeededTrackFinderData/ITkSiSpacePointForSeed.h"

#include "ActsInterop/TableUtils.h"

namespace ActsTrk {
  SeedingAlg::SeedingAlg( const std::string &name,
			  ISvcLocator *pSvcLocator )
    : AthReentrantAlgorithm( name,pSvcLocator ) 
  {}
  
  StatusCode SeedingAlg::initialize() {
    ATH_MSG_INFO( "Initializing " << name() << " ... " );
    if (m_fastTracking)
      ATH_MSG_INFO( "   using fast tracking configuration.");
    
    // Retrieve seed tool
    ATH_CHECK( m_seedsTool.retrieve() );
    ATH_CHECK( m_paramEstimationTool.retrieve() );
    ATH_CHECK( m_trackingGeometryTool.retrieve() );
    ATH_CHECK( m_ATLASConverterTool.retrieve() );

    // Cond
    ATH_CHECK( m_beamSpotKey.initialize() );
    ATH_CHECK( m_fieldCondObjInputKey.initialize() );
    ATH_CHECK( m_detEleCollKey.initialize() );

    // Read and Write handles
    ATH_CHECK( m_spacePointKey.initialize() );
    ATH_CHECK( m_seedKey.initialize() );
    ATH_CHECK( m_actsTrackParamsKey.initialize() );

    ATH_CHECK( m_monTool.retrieve(EnableTool{not m_monTool.empty()}) );

    return StatusCode::SUCCESS;
  }

  StatusCode SeedingAlg::finalize() {
     ATH_MSG_INFO("Seed statistics" << std::endl << makeTable(m_stat,
                                                              std::array<std::string, kNStat>{
                                                                 "Spacepoints",
                                                                 "Seeds",
                                                                 "No track parameters"
                                                                    }).columnWidth(10));
    return StatusCode::SUCCESS;
  }

  StatusCode SeedingAlg::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG( "Executing " << name() <<" ... " );

    auto timer = Monitored::Timer<std::chrono::milliseconds>( "TIME_execute" );
    auto time_seedCreation = Monitored::Timer<std::chrono::milliseconds>( "TIME_seedCreation" );
    auto time_parameterEstimation = Monitored::Timer<std::chrono::milliseconds>( "TIME_parameterEstimation" );
    auto mon = Monitored::Group( m_monTool, timer, time_seedCreation, time_parameterEstimation );

    // ================================================== // 
    // ===================== OUTPUTS ==================== //
    // ================================================== // 
    
    SG::WriteHandle< ActsTrk::SeedContainer > seedHandle = SG::makeHandle( m_seedKey, ctx );
    ATH_MSG_DEBUG( "    \\__ Seed Container `" << m_seedKey.key() << "` created ..." );
    ATH_CHECK( seedHandle.record( std::make_unique< ActsTrk::SeedContainer >() ) );
    ActsTrk::SeedContainer *seedPtrs = seedHandle.ptr();
    
    SG::WriteHandle< ActsTrk::BoundTrackParametersContainer > boundTrackParamsHandle = SG::makeHandle( m_actsTrackParamsKey, ctx );
    ATH_MSG_DEBUG( "    \\__ Track Params Estimated `"<< m_actsTrackParamsKey.key() << "` created ..." );
    ATH_CHECK( boundTrackParamsHandle.record( std::make_unique< ActsTrk::BoundTrackParametersContainer >() ) );
    ActsTrk::BoundTrackParametersContainer *trackParams = boundTrackParamsHandle.ptr();
    
    // ================================================== //
    // ===================== INPUTS ===================== // 
    // ================================================== //
    
    // Read the Beam Spot information
    SG::ReadCondHandle<InDet::BeamSpotData> beamSpotHandle { m_beamSpotKey, ctx };
    ATH_CHECK( beamSpotHandle.isValid() );
    if (beamSpotHandle.cptr() == nullptr) {
      ATH_MSG_ERROR("Retrieved Beam Spot Handle contains a nullptr");
      return StatusCode::FAILURE;
    }
    auto beamSpotData = beamSpotHandle.cptr();
    // Beam Spot Position
    Acts::Vector3 beamPos( beamSpotData->beamPos().x() * Acts::UnitConstants::mm,
                           beamSpotData->beamPos().y() * Acts::UnitConstants::mm,
                           beamSpotData->beamPos().z() * Acts::UnitConstants::mm);
    
    
    ATH_MSG_DEBUG( "Retrieving elements from " << m_spacePointKey.size() << " input collections...");
    std::vector<const xAOD::SpacePointContainer *> all_input_collections;
    all_input_collections.reserve(m_spacePointKey.size());
    
    std::size_t number_input_space_points = 0;
    for (const auto& spacePointKey : m_spacePointKey) {
      ATH_MSG_DEBUG( "Retrieving from Input Collection '" << spacePointKey.key() << "' ..." );
      SG::ReadHandle< xAOD::SpacePointContainer > handle = SG::makeHandle( spacePointKey, ctx );
      ATH_CHECK( handle.isValid() );
      all_input_collections.push_back(handle.cptr());
      ATH_MSG_DEBUG( "    \\__ " << handle->size() << " elements!");
      number_input_space_points += handle->size();
    }
    
    // Apply selection on which SPs you want to use from the input container
    std::vector<const xAOD::SpacePoint*> selectedSpacePoints;
    selectedSpacePoints.reserve(number_input_space_points);
    
    for (const auto* collection : all_input_collections) {
      for (const auto* sp : *collection) {
        if (m_fastTracking and skipSpacePoint(sp->x()-beamPos.x(), sp->y()-beamPos.y(), sp->z()-beamPos.z()))
          continue;
        selectedSpacePoints.push_back( sp );
      }
    }
    
    ATH_MSG_DEBUG( "    \\__ Total input space points: " << selectedSpacePoints.size());
    m_stat[kNSpacepoints] += selectedSpacePoints.size();
    
    // Early Exit in case no space points at this stage
    if (selectedSpacePoints.empty()) {
      ATH_MSG_DEBUG("No input space points found, we stop seeding");
      return StatusCode::SUCCESS;
    }
    
    // ================================================== //
    // ===================== CONDS ====================== // 
    // ================================================== //
        
    // Read the b-field information
    SG::ReadCondHandle<AtlasFieldCacheCondObj> readHandle { m_fieldCondObjInputKey, ctx };
    ATH_CHECK( readHandle.isValid() );
    
    const AtlasFieldCacheCondObj* fieldCondObj{ *readHandle };
    if (fieldCondObj == nullptr) {
      ATH_MSG_ERROR("Failed to retrieve AtlasFieldCacheCondObj with key " << m_fieldCondObjInputKey.key());
      return StatusCode::FAILURE;
    }
    
    // Get the magnetic field
    // Using ACTS classes in order to be sure we are consistent
    Acts::MagneticFieldContext magFieldContext(fieldCondObj);
    ATLASMagneticFieldWrapper magneticField;
    Acts::MagneticFieldProvider::Cache magFieldCache = magneticField.makeCache( magFieldContext );
    Acts::Vector3 bField = *magneticField.getField( Acts::Vector3(beamPos.x(), beamPos.y(), 0),
                                                    magFieldCache );

    SG::ReadCondHandle< InDetDD::SiDetectorElementCollection > detEleHandle( m_detEleCollKey, ctx );
    ATH_CHECK( detEleHandle.isValid() );
    const InDetDD::SiDetectorElementCollection* detEle = detEleHandle.retrieve();
    if ( detEle == nullptr ) {
      ATH_MSG_FATAL( m_detEleCollKey.fullKey() << " is not available." );
      return StatusCode::FAILURE;
    }
    

    // ================================================== // 
    // ===================== COMPUTATION ================ //
    // ================================================== // 

    ATH_MSG_DEBUG("Running Seed Finding ...");    
    time_seedCreation.start();
    ATH_CHECK( m_seedsTool->createSeeds( ctx, 
					 selectedSpacePoints,
					 beamPos,
					 bField,
					 *seedPtrs ) );
    time_seedCreation.stop();
    ATH_MSG_DEBUG("    \\__ Created " << seedPtrs->size() << " seeds");
    m_stat[kNSeeds] += seedPtrs->size();

    // ================================================== //   
    // ================ PARAMS ESTIMATION =============== //  
    // ================================================== //   

    ATH_MSG_DEBUG( "Estimating Track Parameters from seed ..." );

    // Estimate Track Parameters
    auto retrieveSurfaceFunction = 
      [this, &detEle] (const Acts::Seed<xAOD::SpacePoint>& seed) -> const Acts::Surface& 
      { 
	const InDetDD::SiDetectorElement* Element = detEle->getDetectorElement(seed.sp().front()->elementIdList()[0]);
	const Trk::Surface& atlas_surface = Element->surface();
	return this->m_ATLASConverterTool->trkSurfaceToActsSurface(atlas_surface); 
      };

    auto geo_context = m_trackingGeometryTool->getNominalGeometryContext();

    time_parameterEstimation.start();
    for (const ActsTrk::Seed* seed : *seedPtrs) {
      std::optional<Acts::BoundTrackParameters> optTrackParams =
        m_paramEstimationTool->estimateTrackParameters(ctx,
						       *seed,
						       geo_context.context(),
						       magFieldContext,
						       retrieveSurfaceFunction);

      if ( optTrackParams.has_value() ) {
	Acts::BoundTrackParameters *toAdd = 
	  new Acts::BoundTrackParameters( optTrackParams.value() );
	trackParams->push_back( toAdd );
      }
    }
    m_stat[kNSeedsWithoutParam] += (seedPtrs->size() - trackParams->size() );
    time_parameterEstimation.stop();

    return StatusCode::SUCCESS;
  }
  
} // namespace
