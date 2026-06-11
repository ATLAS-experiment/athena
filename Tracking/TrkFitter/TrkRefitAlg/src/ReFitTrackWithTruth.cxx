/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// ReFitTrackWithTruth.cxx
//   Implementation file for class ReFitTrackWithTruth
///////////////////////////////////////////////////////////////////
// version 1.0 03/10/19 Gabriel Facini (Rel21)
// version 2.0 22/03/23 Andrea Sciandra (Rel22/Rel23)
///////////////////////////////////////////////////////////////////

//SiTBLineFitter includes
#include "TrkRefitAlg/ReFitTrackWithTruth.h"

// Gaudi includes
#include "GaudiKernel/TypeNameString.h"

#include "AtlasDetDescr/AtlasDetectorID.h"
#include "TrkRIO_OnTrack/RIO_OnTrack.h"
#include "TrkEventPrimitives/ParamDefs.h"
#include "TrkEventPrimitives/LocalParameters.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkMeasurementBase/MeasurementBase.h"
#include "TrkTrackSummary/TrackSummary.h"
#include "TrkFitterInterfaces/ITrackFitter.h"
#include "TrkToolInterfaces/ITrackSummaryTool.h"
#include "InDetRIO_OnTrack/PixelClusterOnTrack.h"
#include "TruthUtils/MagicNumbers.h"
#include "TRandom3.h"

#include <vector>

// Constructor with parameters:
Trk::ReFitTrackWithTruth::ReFitTrackWithTruth(const std::string &name, ISvcLocator *pSvcLocator) :
  AthAlgorithm(name,pSvcLocator)
{

}

// Initialize method:
StatusCode Trk::ReFitTrackWithTruth::initialize()
{
  ATH_MSG_INFO ("ReFitTrackWithTruth::initialize()");

  ATH_CHECK (m_siHitCollectionName.initialize());
  ATH_CHECK (m_SDOContainerName.initialize());
  ATH_CHECK (m_truthMapName.initialize());

  // get tracks
  ATH_CHECK( m_inputTrackColName.initialize() );

  if (m_ITrackFitter.retrieve().isFailure()) {
    ATH_MSG_FATAL ("Failed to retrieve tool "<<m_ITrackFitter.typeAndName());
    return StatusCode::FAILURE;
  } else {
    ATH_MSG_INFO ("Retrieved general fitter " << m_ITrackFitter.typeAndName());
  }

  if (m_trkSummaryTool.retrieve().isFailure()) {
    ATH_MSG_FATAL ("Failed to retrieve tool " << m_trkSummaryTool);
    return StatusCode::FAILURE;
  } else {
    ATH_MSG_INFO ("Retrieved tool " << m_trkSummaryTool.typeAndName());
  }

  // needed if want to check on shared clusters
  if ( m_assoTool.retrieve().isFailure() ) {
    ATH_MSG_FATAL ("Failed to retrieve tool " << m_assoTool);
    return StatusCode::FAILURE;
  } else ATH_MSG_INFO("Retrieved tool " << m_assoTool);


  // Configuration of the material effects
  m_ParticleHypothesis = Trk::ParticleSwitcher::particle[m_matEffects];

  // Get ID Helper
  ATH_CHECK(detStore()->retrieve(m_idHelper, "AtlasID"));

  // Get Pixel Helper
  if (detStore()->retrieve(m_pixelID, "PixelID").isFailure()) {
    ATH_MSG_FATAL ("Could not get Pixel ID helper");
    return StatusCode::FAILURE;
  }

  //set seed for random-number generator
  m_random->SetSeed();

  // ensure the vector is the correct size in case the user does not want to smear
  if( m_resolutionRPhi.size() < 4 ) { m_resolutionRPhi={0.,0.,0.,0.}; }
  if( m_resolutionZ.size() < 4 ) { m_resolutionZ={0.,0.,0.,0.}; }

  if( m_errorRPhi.size() < 4 ) { m_errorRPhi={1.,1.,1.,1.}; }
  if( m_errorZ.size() < 4 ) { m_errorZ={1.,1.,1.,1.}; }

  ATH_MSG_INFO (" Resolutions " << m_resolutionRPhi.size() << " " << m_resolutionZ.size());
  for( unsigned int i=0; i<m_resolutionRPhi.size(); i++) {
    ATH_MSG_INFO (" " << i << " " << m_resolutionRPhi[i] << "\t" << m_resolutionZ[i]);
  }
  ATH_MSG_INFO (" Error SR " << m_errorRPhi.size() << " " << m_errorZ.size());
  for( unsigned int i=0; i<m_errorRPhi.size(); i++) {
    ATH_MSG_INFO (" " << i << " " << m_errorRPhi[i] << "\t" << m_errorZ[i]);
  }

  ATH_CHECK(m_outputTrackCollectionName.initialize());
  ATH_MSG_DEBUG("m_outputTrackCollectionName: " << m_outputTrackCollectionName);

  return StatusCode::SUCCESS;
}

// Execute method:
StatusCode Trk::ReFitTrackWithTruth::execute(const EventContext& ctx)
{
  ATH_MSG_DEBUG ("ReFitTrackWithTruth::execute()");
  std::unique_ptr<Trk::PRDtoTrackMap> prd_to_track_map(m_assoTool->createPRDtoTrackMap());

  SG::ReadHandle<TrackCollection> tracks(m_inputTrackColName, ctx);
  if (!tracks.isValid()) {
    ATH_MSG_ERROR(m_inputTrackColName.key() << " not found");
    return StatusCode::FAILURE;
  }

  //retrieve truth information needed (SiHitCollection)
  SG::ReadHandle<SiHitCollection> siHits(m_siHitCollectionName, ctx);
  if (!siHits.isValid()) {
    ATH_MSG_WARNING( "Error retrieving SiHitCollection " << m_siHitCollectionName);
    return StatusCode::FAILURE;
  }

  //retrieve truth information needed (SDOCollection)
  SG::ReadHandle<InDetSimDataCollection> sdoCollection(m_SDOContainerName, ctx);
  if (!sdoCollection.isValid()) {
    ATH_MSG_WARNING( "Error retrieving SDOCollection " << m_SDOContainerName );
    return StatusCode::FAILURE;
  }

  //retrieve truth map
  SG::ReadHandle<TrackTruthCollection> truthMap(m_truthMapName, ctx);
  if (!truthMap.isValid()) {
    ATH_MSG_WARNING( "Error retrieving truth map " << m_truthMapName );
    return StatusCode::FAILURE;
  }

  // create new collection of tracks to write in storegate
  std::vector<std::unique_ptr<Trk::Track> > newtracks;

  // loop over tracks
  for (TrackCollection::const_iterator itr  = tracks->begin(); itr != tracks->end(); ++itr) {
    ATH_MSG_DEBUG("input track");

    // Get original parameters
    const TrackParameters* origPerigee = (*itr)->perigeeParameters();
    double od0(0);
    double oz0(0);
    double ophi0(0);
    double otheta(0);
    double oqOverP(0);
    if (!origPerigee){
      ATH_MSG_WARNING("Cannot get original parameters");
    }
    else if (msgLvl(MSG::DEBUG)) {
      od0 = origPerigee->parameters()[Trk::d0];
      oz0 = origPerigee->parameters()[Trk::z0];
      ophi0 = origPerigee->parameters()[Trk::phi0];
      otheta = origPerigee->parameters()[Trk::theta];
      oqOverP = origPerigee->parameters()[Trk::qOverP];
      ATH_MSG_DEBUG ("Original parameters " << od0  << " " << oz0  << " " << ophi0 << " " << otheta << " " << oqOverP);
    }

    // get the unique ID of truth particle matched to the track
    float minProb(0);

    // copy from DenseEnvironmentsAmbiguityProcessorTool.cxx
    ElementLink<TrackCollection> tracklink;
    tracklink.setElement(const_cast<Trk::Track*>(*itr));
    tracklink.setStorableObject(*tracks);
    const ElementLink<TrackCollection> tracklink2=tracklink;

    TrackTruthCollection::const_iterator found = truthMap->find(tracklink2);
    if ( found == truthMap->end() )                { continue; }
    if ( !found->second.particleLink().isValid() ) { continue; }
    if ( found->second.probability() < minProb )   { continue; }
    int uniqueIdToMatch = HepMC::uniqueID(&(found->second.particleLink()));
    ATH_MSG_DEBUG ("Unique ID to match " << uniqueIdToMatch);

    // now that have the track, loop through and build a new set of measurements for a track fit
    std::vector<const Trk::MeasurementBase*> measurementSet;
    std::vector<const Trk::MeasurementBase*> trash;

    ATH_MSG_DEBUG ("Loop over measurementsOnTrack " << (*itr)->measurementsOnTrack()->size() );
    for (const auto *measurement : *((*itr)->measurementsOnTrack())) {
      ATH_MSG_DEBUG ("Next Measurement: " << *measurement);

      // Get measurement as RIO_OnTrack
      const Trk::RIO_OnTrack* rio = dynamic_cast <const Trk::RIO_OnTrack*>( measurement );
      if (rio == nullptr) {
        ATH_MSG_WARNING("Cannot get RIO_OnTrack from measurement. Hit will NOT be included in track fit.");
        continue;
      }

      // if not a pixel cluster, keep the measurement as is and press on
      const Identifier& surfaceID = (rio->identify()) ;
      if( !m_idHelper->is_pixel(surfaceID) ) {
        measurementSet.push_back( measurement );
        continue;
      }

      // only PIXEL CLUSTERS from here on
      const InDet::PixelCluster* pix = dynamic_cast<const InDet::PixelCluster*>(rio->prepRawData());
      if (pix == nullptr) {
        ATH_MSG_WARNING("Cannot get PixelCluster from RIO_OnTrack");
        continue;
      }

      const InDetDD::SiDetectorElement* element = pix->detectorElement();
      const InDetDD::SiDetectorDesign* design = static_cast<const InDetDD::SiDetectorDesign*>(&element->design());
      // To locate cluster module
      Identifier clusterId = pix->identify();
      int bec              = m_pixelID->barrel_ec(clusterId);
      int layer_disk       = m_pixelID->layer_disk(clusterId);

      // Do any SDOs in reconstructed cluster match uniqueIdToMatch
      // Should always return true for pseudotracks. For reco tracks could differ
      bool hasSDOMatch = IsClusterFromTruth( pix, uniqueIdToMatch, *sdoCollection );

      // --- hit flags' logic
      // m_saveWrongHits - only has impact on reco track, where hit might be from wrong particle
      // m_fixWrongHits - fix will only work on hits that are not noise, noise hits behavior unchanged by this flag
      // m_rejNoiseHits - if saving but don't want to save these

      // could be used already here to correct wrong hits
      double maxEnergyDeposit(-1);
      SiHit maxEDepSiHit;

      if ( !hasSDOMatch ) {
        ATH_MSG_DEBUG ("No SDO matching cluster");
        // save wrong hits
        if (not m_saveWrongHits) continue;
        const std::vector<SiHit> matchedSiHits = matchSiHitsToCluster( -999 , pix, siHits );
        if( matchedSiHits.empty() ) {  // then noise, and go to next hit
          if( !m_rejNoiseHits ) { measurementSet.push_back( measurement ); }
          continue;
        } // is a real hit, just not from the particle in question -- will NOT go to next hit just yet
        if( m_fixWrongHits ) { // if fix, use highest energy truth hit
          for( const auto& siHit : matchedSiHits ) {
            if (siHit.energyLoss() > maxEnergyDeposit) {
              maxEnergyDeposit = siHit.energyLoss();
              maxEDepSiHit = siHit;
            }
          }
        } else { // save the wrong hit as is and go to next cluster
          measurementSet.push_back( measurement );
          continue;
        }
      } else { // hasSDOMatch
        // Get All SiHits truth matched to the reconstruction cluster for the particle associated with the track
        const std::vector<SiHit> matchedSiHits = matchSiHitsToCluster( uniqueIdToMatch, pix, siHits );
        if( matchedSiHits.empty() ) {
          ATH_MSG_WARNING ("No SiHit matching cluster");
          continue;  // should NOT HAPPEN for pseudotracks
        }
        ATH_MSG_DEBUG ("N SiHit matching cluster: " << matchedSiHits.size());

        // If multiple SiHits / cluster FOR THE SAME TRUTH PARTICLE,
        // Take position of SiHit giving the most energy
        for( const auto& siHit : matchedSiHits ) {
          if (siHit.energyLoss() > maxEnergyDeposit) {
            maxEnergyDeposit = siHit.energyLoss();
            maxEDepSiHit = siHit;
          }
        }
      } // if else hasSDOMatch

      // - Retrieve true position of cluster from entry/exit point of truth particle
      //get average position of entry/exit point
      HepGeom::Point3D<double> averagePosition = (maxEDepSiHit.localStartPosition() + maxEDepSiHit.localEndPosition()) * 0.5;
      ATH_MSG_DEBUG (" Average position : " << averagePosition);

      // SiHit coordinate system i.e. localStartPosition
      // z = eta -> this is y in ATLAS sensor local frame
      // y = phi -> this is x in ATLAS sensor local frame
      // USE hitLocalToLocal from SiDetectorElement

      HepGeom::Point3D<double> smearedPosition = smearTruthPosition( averagePosition, bec, layer_disk, design );
      ATH_MSG_DEBUG (" Smeared position : " << smearedPosition );

      auto locparOrig = rio->localParameters();
      ATH_MSG_DEBUG(" Original locpar " << locparOrig);

      Trk::LocalParameters locpar = element->hitLocalToLocal(smearedPosition.z(), smearedPosition.y()); // eta, phi
      ATH_MSG_DEBUG(" locpar " << locpar);

      InDetDD::SiLocalPosition centroid(locpar.get(Trk::loc2), locpar.get(Trk::loc1), 0); // eta, phi, depth
      const Amg::Vector3D& globPos = element->globalPosition(centroid);

      InDet::SiWidth pixWidth = pix->width();
      Amg::MatrixX   cov      = pix->localCovariance();
      // Original code took width / nrows (or columns)* 1/sqrt(12) to check if > 0...
      // only need to check width to test against 0...
      // disk one is layer 0
      if(bec!=0) layer_disk++;
      if(pixWidth.phiR()>0) {
        float error(1.0);
        error = getPhiPosResolution(layer_disk)*getPhiPosErrorFactor(layer_disk);
        cov(0,0) = error*error;
      } else {
        ATH_MSG_WARNING("pixWidth.phiR not > 0");
      }

      if(pixWidth.z()>0) {
        float error(1.0);
        error = getEtaPosResolution(layer_disk)*getEtaPosErrorFactor(layer_disk);
        cov(1,1) = error*error;
      } else {
        ATH_MSG_WARNING("pixWidth.z not > 0");
      }

      auto iH = element->identifyHash();

      InDet::PixelClusterOnTrack* pcot = new InDet::PixelClusterOnTrack(pix,std::move(locpar),std::move(cov),iH,globPos,
          pix->gangedPixel(),
          false);

      if(pcot) {
        measurementSet.push_back( pcot);
        trash.push_back(pcot);
      } else {
        ATH_MSG_WARNING("Could not make new PixelClusterOnTrack");
      }

    } // loop over measurements on track



    ATH_MSG_DEBUG ("Fit new tracks with measurementSet : " << measurementSet.size());
    std::unique_ptr<Trk::Track> newtrack;
    try {
      newtrack = m_ITrackFitter->fit(ctx,
          measurementSet,
          *origPerigee,
          m_runOutlier,
          m_ParticleHypothesis);
    }
    catch(const std::exception& e) {
      ATH_MSG_ERROR ("Refit Logic Error. No new track. Message: " << e.what());
      newtrack = nullptr;
    }

    ATH_MSG_DEBUG ("Track fit is done!");

    if (msgLvl(MSG::DEBUG)) {
      if (!newtrack) { ATH_MSG_DEBUG ("Refit Failed"); }
      else {

        ATH_MSG_VERBOSE ("re-fitted track:" << *newtrack);
        const Trk::Perigee* aMeasPer = newtrack->perigeeParameters();
        if (aMeasPer==nullptr){
          ATH_MSG_ERROR ("Could not get Trk::MeasuredPerigee");
        } else {
          double d0 = aMeasPer->parameters()[Trk::d0];
          double z0 = aMeasPer->parameters()[Trk::z0];
          double phi0 = aMeasPer->parameters()[Trk::phi0];
          double theta = aMeasPer->parameters()[Trk::theta];
          double qOverP = aMeasPer->parameters()[Trk::qOverP];
          ATH_MSG_DEBUG ("Refitted parameters differences "
              << (od0-d0)/od0  << " "
              << (oz0-z0)/oz0  << " "
              << (ophi0-phi0)/ophi0 << " "
              << (otheta-theta)/otheta << " "
              << (oqOverP-qOverP)/oqOverP );
        } // aMeasPer exists
      } // newtrack exists
    } // if debug

    if (newtrack) { newtracks.push_back(std::move(newtrack)); }
    else          { ATH_MSG_WARNING ("Refit Failed"); }

  } // loop over tracks

  ATH_MSG_VERBOSE ("Add PRDs to assoc tool.");

  // recreate the summaries on the final track collection with correct PRD tool
  for(const std::unique_ptr<Trk::Track> &new_track : newtracks ) {
    if((m_assoTool->addPRDs(*prd_to_track_map, *new_track)).isFailure()) {ATH_MSG_WARNING("Failed to add PRDs to map");}
  }

  ATH_MSG_VERBOSE ("Recalculate the summary");
  // and copy tracks from vector of non-const tracks to collection of const tracks
  std::unique_ptr<TrackCollection> new_track_collection = std::make_unique<TrackCollection>();
  new_track_collection->reserve(newtracks.size());
  for(std::unique_ptr<Trk::Track> &new_track : newtracks ) {
    m_trkSummaryTool->computeAndReplaceTrackSummary(ctx, *new_track, false /* DO NOT suppress hole search*/);
    new_track_collection->push_back(std::move(new_track));
  }

  ATH_MSG_VERBOSE ("Save tracks");
  ATH_CHECK(SG::WriteHandle<TrackCollection>(m_outputTrackCollectionName, ctx).record(std::move(new_track_collection)));

  ATH_MSG_INFO ("ReFitTrackWithTruth::execute() completed");
  return StatusCode::SUCCESS;
}

std::vector<SiHit> Trk::ReFitTrackWithTruth::matchSiHitsToCluster( const int uniqueIdToMatch,
    const InDet::PixelCluster* pixClus,
    SG::ReadHandle<AtlasHitsVector<SiHit>> &siHitCollection) const {

  // passing a negative unique ID value skip this requirement - can get multiple SiHits upon return from different particles

  ATH_MSG_VERBOSE( " Have " << (*siHitCollection).size() << " SiHits to look through" );
  std::vector<SiHit>  matchingHits;

  // Check if we have detector element  --  needed to find the local position of the SiHits
  const InDetDD::SiDetectorElement* de = pixClus->detectorElement();
  if(!de) {
    ATH_MSG_WARNING("Do not have detector element to find the local position of SiHits!");
    return matchingHits;
  }

  // To locate cluster module
  Identifier clusterId = pixClus->identify();

  std::vector<const SiHit* >  multiMatchingHits;

  // match SiHits to unique ID and make sure in same module as reco hit
  for ( const auto&  siHit : *siHitCollection) {

    if ( uniqueIdToMatch > 0 ) { // negative uniqueIdToMatch will keep all
      if ( HepMC::uniqueID(&(siHit.particleLink())) != uniqueIdToMatch ) { continue; }
    }

    // Check if it is a Pixel hit
    if( !siHit.isPixel() ) { continue; }

    // Match to the cluster module
    if( m_pixelID->barrel_ec(clusterId) != siHit.getBarrelEndcap() ) { continue; }
    if( m_pixelID->layer_disk(clusterId)!= siHit.getLayerDisk() )    { continue; }
    if( m_pixelID->phi_module(clusterId)!= siHit.getPhiModule() )    { continue; }
    if( m_pixelID->eta_module(clusterId)!= siHit.getEtaModule() )    { continue; }

    // Have SiHits in the same module as the cluster at this point
    ATH_MSG_DEBUG("Hit is on the same module");
    multiMatchingHits.push_back(&siHit);

  } // loop over SiHitCollection


  //Now we will now make 1 SiHit for each true particle if the SiHits "touch" other
  std::vector<const SiHit* >::iterator siHitIter  = multiMatchingHits.begin();
  std::vector<const SiHit* >::iterator siHitIter2 = multiMatchingHits.begin();
  ATH_MSG_DEBUG( "Found " << multiMatchingHits.size() << " SiHit " );

  // double loop - for each matching SiHit, consider all the SiHits _next_ in the collection
  // to see if they overlap.
  // if overlapping, combine and only consider new merged hits
  for ( ; siHitIter != multiMatchingHits.end(); ++siHitIter) {
    const SiHit* lowestXPos  = *siHitIter;
    const SiHit* highestXPos = *siHitIter;


    // We will merge these hits
    std::vector<const SiHit* > ajoiningHits;
    ajoiningHits.push_back( *siHitIter );

    siHitIter2 = siHitIter+1;
    while ( siHitIter2 != multiMatchingHits.end() ) {
      // Need to come from the same truth particle

      // wasn't the unique ID match already done!?
      if ( HepMC::uniqueID(&((*siHitIter)->particleLink())) != HepMC::uniqueID(&((*siHitIter2)->particleLink()))) {
        ++siHitIter2;
        continue;
      }

      // Check to see if the SiHits are compatible with each other.
      if (std::abs((highestXPos->localEndPosition().x()-(*siHitIter2)->localStartPosition().x()))<0.00005 &&
          std::abs((highestXPos->localEndPosition().y()-(*siHitIter2)->localStartPosition().y()))<0.00005 &&
          std::abs((highestXPos->localEndPosition().z()-(*siHitIter2)->localStartPosition().z()))<0.00005 )
      {
        highestXPos = *siHitIter2;
        ajoiningHits.push_back( *siHitIter2 );
        // Dont use hit  more than once
        siHitIter2 = multiMatchingHits.erase( siHitIter2 );
        //--siHitIter2; // maybe
      }else if (std::abs((lowestXPos->localStartPosition().x()-(*siHitIter2)->localEndPosition().x()))<0.00005 &&
          std::abs((lowestXPos->localStartPosition().y()-(*siHitIter2)->localEndPosition().y()))<0.00005 &&
          std::abs((lowestXPos->localStartPosition().z()-(*siHitIter2)->localEndPosition().z()))<0.00005)
      {
        lowestXPos = *siHitIter2;
        ajoiningHits.push_back( *siHitIter2 );
        // Dont use hit  more than once
        siHitIter2 = multiMatchingHits.erase( siHitIter2 );
       // --siHitIter2; // maybe
      } else {
        ++siHitIter2;
      }
    } // loop over matching SiHits to see if any overlap

    if( ajoiningHits.empty()){
      ATH_MSG_WARNING("This should really never happen");
      continue;
    }
    if(ajoiningHits.size() == 1){
      // Copy Si Hit ready to return
      matchingHits.push_back( *ajoiningHits[0] );
      continue;
    }
    //  Build new SiHit and merge information together.
    ATH_MSG_DEBUG("Merging " << ajoiningHits.size() << " SiHits together." );


    float energyDep(0);
    float time(0);
    for( auto& siHit :  ajoiningHits){
      energyDep += siHit->energyLoss();
      time += siHit->meanTime();
    }
    time /= (float)ajoiningHits.size();

    matchingHits.emplace_back(lowestXPos->localStartPosition(),
                              highestXPos->localEndPosition(),
                              energyDep,
                              time,
                              HepMC::uniqueID(&((*siHitIter)->particleLink())),
                              0, // 0 for pixel 1 for Pixel
                              (*siHitIter)->getBarrelEndcap(),
                              (*siHitIter)->getLayerDisk(),
                              (*siHitIter)->getEtaModule(),
                              (*siHitIter)->getPhiModule(),
                              (*siHitIter)->getSide() );
    ATH_MSG_DEBUG("Finished Merging " << ajoiningHits.size() << " SiHits together." );
  } // loop over all matching SiHits

  return matchingHits;
}

bool Trk::ReFitTrackWithTruth::IsClusterFromTruth( const InDet::PixelCluster* pixClus,
    const int uniqueIdToMatch,
    const InDetSimDataCollection &sdoCollection) {

  // Should be true for all pseudotracks
  // Can be false for reco tracks - misassigned hits

  bool match(false);

  // loop over reconstructed energy depoists in the cluster
  for( const auto &hitIdentifier : pixClus->rdoList() ) {

    // find the rdo in the sdo collection
    auto pos = sdoCollection.find(hitIdentifier);
    if( pos == sdoCollection.end() ) { continue; }

    // get the unique ID from each deposit
    for( const auto& deposit : pos->second.getdeposits() ){
      if( !deposit.first ){ continue; } // if truthparticle(?) link doesn't exists? Energy deposit is still known
      if( HepMC::uniqueID(&(deposit.first)) != uniqueIdToMatch ) { continue; }
      match = true;
      break;
    }
    if(match) { break; }
  }

  return match;
}

HepGeom::Point3D<double> Trk::ReFitTrackWithTruth::smearTruthPosition( const HepGeom::Point3D<double>& orig,
    const int bec,
    const int layer_disk,
    const InDetDD::SiDetectorDesign* design) const {

  HepGeom::Point3D<double> smeared(0,0,0);

  smeared.setX(orig.x());

  if (bec == 0) {
    double smearLocY = m_random->Gaus(0, getPhiPosResolution(layer_disk));
    double smearLocZ = m_random->Gaus(0, getEtaPosResolution(layer_disk));
    smeared.setY(orig.y() + smearLocY);
    smeared.setZ(orig.z() + smearLocZ);

  } else {

    smeared.setY(orig.y());
    smeared.setZ(orig.z());
  }

  //check for module boundaries
  if (smeared.y()>design->width()/2) {
    smeared.setY(design->width()/2-1e-6);
  } else if (smeared.y()<-design->width()/2) {
    smeared.setY(-design->width()/2+1e-6);
  }
  if (smeared.z()>design->length()/2) {
    smeared.setZ(design->length()/2-1e-6);
  } else if (smeared.z()<-design->length()/2) {
    smeared.setZ(-design->width()/2+1e-6);
  }


  return smeared;
}

double Trk::ReFitTrackWithTruth::getPhiPosResolution(int layer) const { return m_resolutionRPhi[layer]; }
double Trk::ReFitTrackWithTruth::getEtaPosResolution(int layer) const { return m_resolutionZ[layer];    }

double Trk::ReFitTrackWithTruth::getPhiPosErrorFactor(int layer) const { return m_errorRPhi[layer]; }
double Trk::ReFitTrackWithTruth::getEtaPosErrorFactor(int layer) const { return m_errorZ[layer];    }
