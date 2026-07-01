/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////
// TruthTrackBuilder.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

// package include
#include "TruthTrackBuilder.h"
// Trk includes
#include "TrkTrack/Track.h"

#include "TrkSurfaces/PerigeeSurface.h"
#include "TrkPrepRawData/PrepRawData.h"
#include "TrkRIO_OnTrack/RIO_OnTrack.h"
// Gaudi
#include "GaudiKernel/SystemOfUnits.h"
// HepMC
#include "AtlasHepMC/GenParticle.h"
#include "AtlasHepMC/GenVertex.h"
#include "TruthUtils/MagicNumbers.h"
#include "TruthUtils/HepMCHelpers.h"

#include "AtlasDetDescr/AtlasDetectorID.h"

/** Constructor **/
Trk::TruthTrackBuilder::TruthTrackBuilder(const std::string& t, const std::string& n, const IInterface* p) : 
  AthAlgTool(t,n,p),
  m_DetID(nullptr)
{
    declareInterface<Trk::ITruthTrackBuilder>(this);
}


// Athena algtool's Hooks
StatusCode  Trk::TruthTrackBuilder::initialize()
{
    ATH_MSG_VERBOSE("Initializing ...");
    if (m_rotcreator.retrieve().isFailure()) { 
      msg(MSG::FATAL) << "Could not get " << m_rotcreator.type() << endmsg;
      return StatusCode::FAILURE; 
    }

    if (m_rotcreatorbroad.retrieve().isFailure()) { 
      msg(MSG::FATAL) << "Could not get " << m_rotcreatorbroad.type() << endmsg;
      return StatusCode::FAILURE; 
    }

    // track fitter
    if (m_trackFitter.retrieve().isFailure()){
        ATH_MSG_ERROR("Could not retrieve " << m_trackFitter << ". Aborting ...");
        return StatusCode::FAILURE;
    }

    // extrapolator
    if (m_extrapolator.retrieve().isFailure()){
        ATH_MSG_ERROR("Could not retrieve " << m_extrapolator << ". Aborting ...");
        return StatusCode::FAILURE;
    }
    
    // need an Atlas id-helper to identify sub-detectors, take the one from detStore
    if (detStore()->retrieve(m_DetID, "AtlasID").isFailure()) {
      ATH_MSG_ERROR ("Could not get AtlasDetectorID helper" );
      return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
}

                                    
StatusCode  Trk::TruthTrackBuilder::finalize()
{
    ATH_MSG_VERBOSE("Finalizing ...");
    return StatusCode::SUCCESS;
}


Trk::Track* Trk::TruthTrackBuilder::createTrack(const PRD_TruthTrajectory& prdTraj, SegmentCollection* segs ) const 
{
    const EventContext& ctx = Gaudi::Hive::currentContext();
    if( segs != nullptr ){
      ATH_MSG_WARNING("Requested to fill segment collection but mode not implemented");
    }
    ATH_MSG_VERBOSE("The PRD Truth trajectory contains " << prdTraj.prds.size() << " PRDs.");

    // get the associated GenParticle
    auto genPart = prdTraj.genParticle;
    if (!genPart) {
        ATH_MSG_WARNING("No GenParticle associated to this PRD_TruthTrajectory. Ignoring track creation.");
        return nullptr;
    }    
    ATH_MSG_VERBOSE("Got PRD Truth trajectory with " << prdTraj.nDoF << " degrees of freedom.");
    // check min degrees of freedom
    if ( m_minNdof > 0 &&  prdTraj.nDoF < m_minNdof) return nullptr;
    // get the startPosition : math library madness as usual
    Amg::Vector3D startPos = genPart->production_vertex() ?
      Amg::Vector3D(genPart->production_vertex()->position().x(),
                            genPart->production_vertex()->position().y(),
		    genPart->production_vertex()->position().z()) : Amg::Vector3D(0.,0.,0.);
    Amg::Vector3D startMom(genPart->momentum().x(),
                                 genPart->momentum().y(),
                                 genPart->momentum().z());
    //!< get the charge via the particle table ...
    
    const int pdgCode = genPart->pdg_id();
    const double charge = MC::charge(pdgCode);

    // ----------------------- get teh PRDS and start 
    const std::vector<const Trk::PrepRawData*> & clusters = prdTraj.prds;
    // nominal 0,0,0 position for track fit seeding
    Trk::PerigeeSurface persurf;
    Trk::CurvilinearParameters startParams(startPos,startMom,charge);
    //minimal conversion; ideally the extrapolator would return a unique_ptr
    auto per = std::unique_ptr<Trk::TrackParameters>(
      m_extrapolator->extrapolate(ctx,
                                  startParams,
                                  persurf,
                                  Trk::anyDirection,
                                  false,
                                  Trk::nonInteracting));
    if (!per) {
      ATH_MSG_DEBUG("Perigee creation for genParticle start position failed. "
                    "Skipping track creation.");
      return nullptr;
    }
    // first TrackStateOnSurface is the Perigee 
    std::bitset<Trk::TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes> typePattern;
    typePattern.set(Trk::TrackStateOnSurface::Perigee);
    
   const Trk::TrackStateOnSurface *pertsos=new Trk::TrackStateOnSurface(nullptr,std::move(per),nullptr,typePattern);
   auto traj = std::make_unique<Trk::TrackStates>();
   traj->push_back(pertsos);
   
   
   
   std::unique_ptr<const Trk::TrackParameters> prevpar(startParams.uniqueClone());
   // First create a Trk::Track object 'traj' that will go into the fitter for refitting
   int i=0;
   for ( ;i<(int)clusters.size();i++){
        if (m_DetID->is_trt(clusters[i]->identify())) break;
        const Trk::Surface &surf=clusters[i]->detectorElement()->surface(clusters[i]->identify());	
        if (surf==prevpar->associatedSurface()) continue;
        bool ispixel=false;
        if (m_DetID->is_pixel(clusters[i]->identify())) ispixel=true;

        auto thispar = std::unique_ptr<const Trk::TrackParameters>(
          m_extrapolator->extrapolate(ctx,
                                      *prevpar,
                                      surf,
                                      Trk::alongMomentum,
                                      false,
                                      Trk::nonInteracting));
        if (!thispar)
          break;
        if (!surf.insideBounds(thispar->localPosition(),20*Gaudi::Units::mm,50*Gaudi::Units::mm)) {
          continue;
        }
        AmgVector(5) params=thispar->parameters();
        params[Trk::loc1]=clusters[i]->localPosition().x();
        if (ispixel) params[Trk::loc2]=clusters[i]->localPosition().y();
        std::bitset<Trk::TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes> typePattern;
        typePattern.set(Trk::TrackStateOnSurface::Measurement);
        std::unique_ptr<Trk::RIO_OnTrack> rot{m_rotcreator->correct(*clusters[i],*thispar,ctx)};        
        if (!rot) {
          continue;
        }
        // create the ROTs for the reference trajectory
        const Trk::TrackStateOnSurface *tsos=new Trk::TrackStateOnSurface(std::move(rot),thispar->uniqueClone(),nullptr,typePattern);
        traj->push_back(tsos);
        prevpar=std::move(thispar);
   }
   // this is the reference trajectory to be refitted  
   Trk::TrackInfo info;
   Trk::Track track(info, std::move(traj), nullptr);
   if (/* ndof<0 */ (track.measurementsOnTrack()->size() < m_minSiHits &&
                     std::abs(genPart->momentum().eta()) <= m_forwardBoundary) ||
       (track.measurementsOnTrack()->size() < m_minSiHitsForward &&
        std::abs(genPart->momentum().eta()) > m_forwardBoundary) ||
       (m_onlyPrimaries && HepMC::is_simulation_particle(genPart))) {
     ATH_MSG_VERBOSE(
       "Track does not fulfill requirements for refitting. Skipping it.");
     return nullptr;
   }
   // choose the material effects
   //!< @todo : if we need a dedicated electron fitter is has to go in here !
   
   // create the refitted track
   //
   //
   //
   Trk::ParticleHypothesis materialInteractions = Trk::ParticleSwitcher::particle[m_matEffects];

   Trk::Track *refittedtrack=(m_trackFitter->fit(Gaudi::Hive::currentContext(),track,false,materialInteractions)).release();
  
   //!<  Refit a second time to add TRT hits
   Trk::Track *refittedtrack2=nullptr;
   if (refittedtrack && (int)clusters.size()-i>=9){
     
     //owner of measurements so they get cleaned up automatically if needs be
     std::vector<std::unique_ptr<MeasurementBase>> meassetOwn;
     //vector of plain ptr to the owned ones acts as a "view"
     Trk::MeasurementSet measset;
    
     std::unique_ptr<const Trk::TrackParameters> prevpar(refittedtrack->trackParameters()->back()->uniqueClone());
     for (;i<(int)clusters.size();i++) {
       const Trk::Surface *surf=&clusters[i]->detectorElement()->surface(clusters[i]->identify());
       std::unique_ptr<const Trk::TrackParameters> thispar(m_extrapolator->extrapolate(ctx,*prevpar,*surf,Trk::alongMomentum,false,Trk::nonInteracting));
       if (!thispar) break;
       Trk::RIO_OnTrack *rot=m_rotcreatorbroad->correct(*clusters[i],*thispar,ctx);

       if (rot) {
         meassetOwn.emplace_back(rot);
         measset.push_back(meassetOwn.back().get());
       }
       prevpar=std::move(thispar);
     }

     refittedtrack2=(m_trackFitter->fit(Gaudi::Hive::currentContext(),*refittedtrack,measset,false,materialInteractions)).release();

     if (!refittedtrack2){
       auto traj2 = std::make_unique<Trk::TrackStates>();
       for (const Trk::TrackStateOnSurface *j : *refittedtrack->trackStateOnSurfaces()) traj2->push_back(new Trk::TrackStateOnSurface(*j));
       std::bitset<Trk::TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes> typePattern2;
       typePattern2.set(Trk::TrackStateOnSurface::Outlier);
       //measset needs to be unique_ptr before progress further
       for (auto & j : meassetOwn) {
         traj2->push_back(new Trk::TrackStateOnSurface(
           std::move(j),
           nullptr,
           nullptr,
           typePattern2));
       }
       refittedtrack2 = new Trk::Track(refittedtrack->info(),
                                       std::move(traj2),
                                       refittedtrack->fitQuality()->uniqueClone());
     }
   } else if(!refittedtrack){
    ATH_MSG_VERBOSE("Track fit of truth trajectory NOT successful, NO track created. ");
    return nullptr;
   }

   if (refittedtrack2) delete refittedtrack;
   if (!refittedtrack2 && refittedtrack) refittedtrack2=refittedtrack;

   //         
   ATH_MSG_DEBUG("Track fit of truth trajectory successful, track created. ");
   // return what you have
   // Before returning, fix the creator
   if (refittedtrack2){
     refittedtrack2->info().setPatternRecognitionInfo( Trk::TrackInfo::Pseudotracking);
   }
   return refittedtrack2;
}

