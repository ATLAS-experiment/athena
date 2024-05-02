/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):
// $Id$
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "TrkTrack/TrackCollection.h"

// Local include(s):
#include "TrackCollectionCnvTool.h"

namespace xAODMaker {

  TrackCollectionCnvTool::TrackCollectionCnvTool( const std::string& type, 
					      const std::string& name,
					      const IInterface* parent )
    : AthAlgTool( type, name, parent ),
      m_particleCreator("Trk::TrackParticleCreatorTool/TrackParticleCreatorTool")
  {
    
    // Declare the interface(s) provided by the tool:
    declareInterface< ITrackCollectionCnvTool >( this );
    declareProperty("TrackParticleCreator", m_particleCreator, "creator of xAOD::TrackParticles");
  }
  
  StatusCode TrackCollectionCnvTool::initialize() {
    // Greet the user:
    ATH_MSG_INFO( "Initializing TrackCollectionCnvTool with " << m_particleCreator.name() );
    ATH_CHECK( m_particleCreator.retrieve() );
    
    return StatusCode::SUCCESS;
  }

  StatusCode TrackCollectionCnvTool::convert( const TrackCollection* aod,
					      xAOD::TrackParticleContainer* xaod, const xAOD::Vertex* vtx ) const {
    
    ATH_MSG_DEBUG( "Sizes of containers before conversion: aod, xaod: " << aod->size() << ", " << xaod->size() );
    
    TrackCollection::const_iterator itr = aod->begin();
    TrackCollection::const_iterator end = aod->end();

    for( ;itr!=end;++itr ) {
      // Create the xAOD object:
      if (!(*itr)) {
        ATH_MSG_WARNING("Empty element in container!");
        continue;
      }
      xAOD::TrackParticle* particle = createParticle(*xaod, *aod, **itr, vtx);

      if (!particle) {
        ATH_MSG_WARNING("Failed to create a TrackParticle");
        continue;
      }
    }

    ATH_MSG_DEBUG( "Sizes of containers after conversion: aod, xaod: " << aod->size() << ", " << xaod->size() );

    return StatusCode::SUCCESS;    
  }

  StatusCode TrackCollectionCnvTool::convertAndAugment( const TrackCollection* aod,
					      xAOD::TrackParticleContainer* xaod, const ObservedTrackMap* trk_map, const xAOD::Vertex* vtx ) const {
        
    ATH_MSG_DEBUG( "convertAndAugment: Sizes of containers before conversion: aod, xaod: " << aod->size() << ", " << xaod->size() );
    ATH_MSG_DEBUG( "convertAndAugment: Size of track map: " << trk_map->size() );
    
    TrackCollection::const_iterator itr = aod->begin();
    TrackCollection::const_iterator end = aod->end();
    ObservedTrackMap::const_iterator itrMap = trk_map->begin();

    // Check size of track collection matches size of observed tracks map
    if(aod->size() != trk_map->size()){
      ATH_MSG_ERROR("convertAndAugment: Number of tracks different in collection to convert vs. observed tracks map: "<<aod->size()<<" vs. "<<trk_map->size());
      return StatusCode::FAILURE;
    }

    for( ;itr!=end;++itr ) {
      // Create the xAOD object:
      if (!(*itr)) {
        ATH_MSG_WARNING("convertAndAugment: Empty element in container!");
        continue;
      }
      xAOD::TrackParticle* particle = createParticle(*xaod, *aod, **itr, vtx);
      if(!particle){
        ATH_MSG_WARNING("convertAndAugment: Failed to create a TrackParticle");
        ++itrMap;
        continue;
      }
      // Augment xAOD object with information from track map
      static const SG::Decorator<long int> IdDec           ("Id");
      IdDec (*particle)                = (*itrMap).first;

#define DECORATE2(NAME, SEL, TYPE)                                      \
      static const SG::Decorator<TYPE> NAME##Dec (#NAME);               \
        NAME##Dec(*particle) = std::get<xAOD::ObserverToolIndex::SEL>((*itrMap).second)
#define DECORATE(NAME, TYPE) DECORATE2(NAME, NAME, TYPE)
      DECORATE(score, double);
      DECORATE(rejectStep, int);
      DECORATE(rejectReason, int);
      DECORATE(parentId, long int);
      DECORATE(numPixelHoles, int);
      DECORATE(numSCTHoles, int);
      DECORATE(numSplitSharedPixel, int);
      DECORATE(numSplitSharedSCT, int);
      DECORATE(numSharedOrSplit, int);
      DECORATE(numSharedOrSplitPixels, int);
      DECORATE(numShared, int);
      DECORATE(isPatternTrack, int);
      DECORATE(totalSiHits, int);
      DECORATE(inROI, int);
      DECORATE2(thishasblayer, hasIBLHit, int);
      DECORATE2(hassharedblayer, hasSharedIBLHit, int);
      DECORATE2(hassharedpixel, hasSharedPixel, int);
      DECORATE2(firstisshared, firstPixIsShared, int);
      DECORATE(numPixelDeadSensor, int);
      DECORATE(numSCTDeadSensor, int);
      DECORATE(numPixelHits, int);
      DECORATE(numSCTHits, int);
      DECORATE(numUnused, int);
      DECORATE(numTRT_Unused, int);
      DECORATE(numSCT_Unused, int);
      DECORATE(numPseudo, int);
      DECORATE(averageSplit1, int);
      DECORATE(averageSplit2, int);
      DECORATE(numWeightedShared, int);
#undef DECORATE
#undef DECORATE2

      const std::vector<xAOD::RejectionStep>& v_rejectStep =
        std::get<xAOD::ObserverToolIndex::rejectStep_full>((*itrMap).second);
      const std::vector<xAOD::RejectionReason>& v_rejectReason =
        std::get<xAOD::ObserverToolIndex::rejectReason_full>((*itrMap).second);
      static const SG::Decorator<std::vector<int> > rejectStep_fullDec ("rejectStep_full");
      static const SG::Decorator<std::vector<int> > rejectReason_fullDec ("rejectReason_full");
      rejectStep_fullDec(*particle).assign (v_rejectStep.begin(), v_rejectStep.end());
      rejectReason_fullDec(*particle).assign (v_rejectReason.begin(), v_rejectReason.end());
      ATH_MSG_DEBUG("convertAndAugment: Augmenting TrackParticle with id "
                    << IdDec(*particle) << " and rejectReason "
                    << rejectReasonDec(*particle)
                    << " (has chi2 = " << particle->chiSquared() << ")");
      ++itrMap;
    }
    ATH_MSG_DEBUG(
      "convertAndAugment: Sizes of containers after conversion: aod, xaod: "
      << aod->size() << ", " << xaod->size());
    return StatusCode::SUCCESS;    
  }

  xAOD::TrackParticle* TrackCollectionCnvTool::createParticle(xAOD::TrackParticleContainer& xaod,
							      const TrackCollection& container,
							      const Trk::Track& tp,
                    const xAOD::Vertex* vtx) const {
    // create the xAOD::TrackParticle, the pointer is added to the container in the function
    ElementLink<TrackCollection> trackLink( &tp, container );
    return m_particleCreator->createParticle( trackLink, &xaod , vtx);
    //no!    return m_particleCreator->createParticle( tp, &xaod );
  }

  StatusCode TrackCollectionCnvTool::setParticleCreatorTool(ToolHandle<Trk::ITrackParticleCreatorTool> *tool)
  {
    ATH_MSG_DEBUG( "In setParticleCreatorTool" );
    m_particleCreator = *tool;

    return StatusCode::SUCCESS;
  }

} // namespace xAODMaker

