/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TrkTruthCreatorTools/DetailedTrackTruthBuilder.h"
#include "TrkTruthData/DetailedTrackTruthCollection.h"
#include "TrkTruthData/DetailedTrackTruth.h"
#include "TrkTruthData/TruthTrajectory.h"

#include "AtlasHepMC/GenParticle.h"
#include "AtlasHepMC/GenVertex.h"
#include "TruthUtils/HepMCHelpers.h"

#include "GeneratorObjects/HepMcParticleLink.h"
#include "GeneratorObjects/McEventCollection.h"
#include "AthContainers/DataVector.h"

#include "TrkTrack/Track.h"
#include "TrkRIO_OnTrack/RIO_OnTrack.h"
#include "TrkCompetingRIOsOnTrack/CompetingRIOsOnTrack.h"
#include "TrkMeasurementBase/MeasurementBase.h"
#include "TrkPrepRawData/PrepRawData.h"

#include <map>
#include <queue>
#include <list>
#include <limits>

#include <iostream>
#include <iterator>
#include <algorithm>

namespace {

  template<class Map>  void printMap(const Map& m) {
    std::cout<<"printMap(): [";
    for (typename Map::const_iterator i=m.begin(); i!=m.end(); ++i) {
      std::cout<<"("<<i->first<<","<<i->second<<"), ";
    }
    std::cout<<"]"<<std::endl;
  }

  struct SubDetPRDs {
    std::set<Identifier> subDetHits[SubDetHitStatistics::NUM_SUBDETECTORS];
  };

  SubDetPRDs& operator+=(SubDetPRDs& a, const SubDetPRDs& b)  {
    for (unsigned i=0; i<SubDetHitStatistics::NUM_SUBDETECTORS; ++i) {
      const std::set<Identifier>& bset = b.subDetHits[i];
      for (std::set<Identifier>::const_iterator pb=bset.begin(); pb!=bset.end(); ++pb) {
        a.subDetHits[i].insert(*pb);
      }
    }
    return a;
  }

  SubDetHitStatistics makeSubDetHitStatistics(const SubDetPRDs& prds) {
    SubDetHitStatistics res;
    for (unsigned i=0; i<SubDetHitStatistics::NUM_SUBDETECTORS; ++i) {
      res[SubDetHitStatistics::SubDetType(i)] = prds.subDetHits[i].size();
    }
    return res;
  }

  // Truth trajectory sprouts
  class Sprout : public std::list<HepMC::ConstGenParticlePtr> {
  public:
    SubDetPRDs stat;
  };

}


namespace Trk {

 //================================================================
  StatusCode DetailedTrackTruthBuilder::initialize()
  {
    ATH_CHECK( m_truthTrajBuilder.retrieve() );
    ATH_CHECK( detStore()->retrieve(m_idHelper, "AtlasID") );

    return StatusCode::SUCCESS;
  }

  //================================================================
  void DetailedTrackTruthBuilder::
  buildDetailedTrackTruth(DetailedTrackTruthCollection *output,
                          const TrackCollection& tracks,
                          const std::vector<const PRD_MultiTruthCollection*>& prdTruth,
                          const EventContext& ctx) const
  {
    ATH_MSG_VERBOSE("DetailedTrackTruthBuilder::buildDetailedTrackTruth() ");

    if (!output) { return; }

    //----------------------------------------------------------------
    // The caller can pass PRD truth collections in any order. Sort them out.

    std::vector<const PRD_MultiTruthCollection*> orderedPRD_Truth(SubDetHitStatistics::NUM_SUBDETECTORS);
    PRD_InverseTruth inverseTruth;

    for ( const PRD_MultiTruthCollection* prdTruthColl : prdTruth ) {
      if (prdTruthColl->empty()) {
          continue;
      }
        
      SubDetHitStatistics::SubDetType subdet = findSubDetType(prdTruthColl->begin()->first);
      ATH_MSG_DEBUG("Assign: "<<prdTruthColl<<" to "<<subdet);
      if (subdet != SubDetHitStatistics::NUM_SUBDETECTORS) {
         orderedPRD_Truth[subdet] = prdTruthColl;
         makeTruthToRecMap(inverseTruth,*prdTruthColl);
      } else {
        ATH_MSG_WARNING("Got unknown SubDetType in prdTruth ");
      }
    }
    ATH_MSG_DEBUG("makeTruthToRecMap filling done");

    //----------------------------------------------------------------
    // Find associated truth for each track

    for (unsigned itrack=0; itrack<tracks.size(); ++itrack) {
      ElementLink<DataVector<Trk::Track> > ptrack(tracks, itrack);
      addTrack(output, ptrack, orderedPRD_Truth, inverseTruth, ctx);
    }

    // DEBUG
    // FIXME: in normal production jobs that does *a lot of* formatting only to discard the result...
    ATH_MSG_DEBUG("Dumping output collection.\n"
                  " Entries with TruthTrajectories of more then one particle shown at the DEBUG level.\n"
                  " Use VERBOSE level for complete dump.");

    for (DetailedTrackTruthCollection::const_iterator i=output->begin(); i!=output->end(); ++i) {
      bool interesting = (i->second.trajectory().size() > 1);

      msg(interesting  ? MSG::DEBUG : MSG::VERBOSE)
        <<"out: trk="<<i->first.index()<<" => "<<i->second<<endmsg;

      if (interesting) {
        const TruthTrajectory& t = i->second.trajectory();
        msg(MSG::VERBOSE)<<"Particles on the trajectory:\n";
        for (unsigned k=0; k<t.size(); ++k) {
          msg(MSG::VERBOSE)<<t[k]<<"\n";
        }
        msg(MSG::VERBOSE)<<"\n"<<endmsg;
      }

    }

  }


  //================================================================
  // Plagiarized from
  // Tracking/TrkTools/TrkTruthCreatorTools/src/TrackTruthMaker.cxx
  // (no way to reuse its private function PrepRawDataCollectionType() )

  SubDetHitStatistics::SubDetType
  DetailedTrackTruthBuilder::findSubDetType(const Identifier& id) const {
    if (m_idHelper->is_pixel(id))
      return SubDetHitStatistics::Pixel;
    if (m_idHelper->is_sct(id))
      return SubDetHitStatistics::SCT;
    if (m_idHelper->is_trt(id))
      return SubDetHitStatistics::TRT;
    if (m_idHelper->is_mdt(id))
      return SubDetHitStatistics::MDT;
    if (m_idHelper->is_rpc(id))
      return SubDetHitStatistics::RPC;
    if (m_idHelper->is_tgc(id))
      return SubDetHitStatistics::TGC;
    if (m_idHelper->is_stgc(id))
      return SubDetHitStatistics::STGC;
    if (m_idHelper->is_mm(id))
      return SubDetHitStatistics::MM;
    if (m_idHelper->is_csc(id))
      return SubDetHitStatistics::CSC;


    ATH_MSG_WARNING("findSubDetType(): UNKNOWN subdet for id="<<id);

    return SubDetHitStatistics::NUM_SUBDETECTORS;
  }

  namespace {
    class ExtendedEventIndex {
    public:
      ExtendedEventIndex(const HepMcParticleLink &source, IProxyDict *proxy)
        : m_eventIndex(source.eventIndex()),
          m_isPosition(m_eventIndex == HepMcParticleLink::ExtendedBarCode::UNDEFINED)
      {
        if (m_isPosition) {
          m_eventIndex = source.getEventPositionInCollection(proxy);
        }
      }

      HepMcParticleLink makeLink(HepMcParticleLink::barcode_type other_particle_id, IProxyDict *proxy) {
        return {other_particle_id, m_eventIndex, (m_isPosition ? HepMcParticleLink::IS_POSITION : HepMcParticleLink::IS_EVENTNUM ), HepMcParticleLink::IS_ID, proxy};
      }

    private:
      HepMcParticleLink::index_type m_eventIndex;
      bool                          m_isPosition;
    };
  }

  //================================================================
  void DetailedTrackTruthBuilder::addTrack(DetailedTrackTruthCollection *output,
                                           const ElementLink<DataVector<Trk::Track> > &trackLink,
                                           const std::vector<const PRD_MultiTruthCollection*>& orderedPRD_Truth,
                                           const PRD_InverseTruth& inverseTruth,
                                           const EventContext& ctx) const
  {
    const Trk::Track& track{**trackLink};
    SubDetHitStatistics trackStat;
    std::map<HepMcParticleLink,SubDetPRDs> pairStat; // stats for (track,GenParticle) for the current track
    IProxyDict *proxy=ctx.getExtension<Atlas::ExtendedEventContext>().proxy();
    //----------------------------------------------------------------
    //  loop over the RIO_OnTrack
    for (const Trk::MeasurementBase* measurement : *track.measurementsOnTrack()) {
      // not all MB are necessarily ROTs.
      const Trk::RIO_OnTrack * riontrack = dynamic_cast<const Trk::RIO_OnTrack*>(measurement);

      if (!riontrack) { // handle CompetingRIOsOnTrack case
        const Trk::CompetingRIOsOnTrack* competing = dynamic_cast<const Trk::CompetingRIOsOnTrack*>(measurement);
        if (competing) {
          riontrack = & competing->rioOnTrack( competing->indexOfMaxAssignProb() );
        }
      }
      if (!riontrack){
          continue;
      }
      ATH_MSG_VERBOSE("Process ROT "<<(*riontrack));

      // get the PrepRawData from the RIO_OnTrack
      const Trk::PrepRawData* prd = riontrack->prepRawData();
      if (!prd) {
          ATH_MSG_WARNING("ROT without prd found...");
          continue;
      }
      const Identifier& id = prd->identify();
      SubDetHitStatistics::SubDetType subdet = findSubDetType(id);

      if (subdet == SubDetHitStatistics::NUM_SUBDETECTORS) {
          ATH_MSG_WARNING("Invalid detector id "<<id);
          continue;
      }
      if (!orderedPRD_Truth[subdet]) {
          ATH_MSG_VERBOSE("No truth collection registered for "<<subdet);
          continue;
      }
  
      ++trackStat[subdet];

      using iprdt = PRD_MultiTruthCollection::const_iterator;
      std::pair<iprdt, iprdt> range = orderedPRD_Truth[subdet]->equal_range(id);

      int n=0;
      // Loop over particles contributing to this cluster
      for (iprdt i = range.first; i!= range.second; ++i) {
        HepMC::ConstGenParticlePtr pa = (*i).second.cptr();
        if (!pa) { 
          continue; 
        }

        if (!i->second.isValid()) {
          ATH_MSG_WARNING("Unexpected invalid HepMcParticleLink in PRD_MultiTruthCollection");
          continue;
        }
        pairStat[i->second].subDetHits[subdet].insert(id);
        ++n;
        ATH_MSG_VERBOSE("PRD-ID:"<<id<<" subdet:"<<subdet<<" number:"<<n<<" particle link:"<<i->second);
      }
      if (n == 0) {
        ATH_MSG_VERBOSE("--> no link, noise ? PRD-ID:"<<id<<" subdet:"<<subdet);
        // add id 0 to pairs, we like to keep track of fake fakes
        unsigned int UID(HepMC::UNDEFINED_ID);
        unsigned int EV(0);
        pairStat[HepMcParticleLink(UID,EV,HepMcParticleLink::IS_EVENTNUM,HepMcParticleLink::IS_ID)].subDetHits[subdet].insert(id);
      }
       
    } // Loop over measurements
    
    if (msgLvl(MSG::VERBOSE)) {
      msg(MSG::VERBOSE)<<"PRD truth particles = ";
      for (std::map<HepMcParticleLink,SubDetPRDs>::const_iterator i=pairStat.begin(); i!=pairStat.end(); ++i) {
        msg(MSG::VERBOSE)<<i->first<<",";
      }
      msg(MSG::VERBOSE)<<endmsg;
    }

    ATH_MSG_VERBOSE("Construct the stat structures....");
    //----------------------------------------------------------------
    // The stat structures are ready.
    // Build truth trajectories for the track

    std::set<HepMcParticleLink> seeds;
    for (std::map<HepMcParticleLink,SubDetPRDs>::const_iterator i=pairStat.begin(); i!=pairStat.end(); ++i) {
      if (i->first.isValid()) {
        seeds.insert(i->first);
      }
      else {
        // add id 0 particles, we like to keep track of fake fakes
        TruthTrajectory traj;
        traj.reserve(1);
        traj.push_back(i->first);
        ATH_MSG_VERBOSE("addTrack(): add id 0 hits (noise ?) to DetailedTrackTruthCollection.");

        // noise/no truth hits on this track
        SubDetHitStatistics noiseStat = makeSubDetHitStatistics(i->second);

        // Only valid HepMcParticleLink make it into seeds and then into sprouts, and
        // stored in the loop over sprouts below.
        // Store output for noise/no truth particles here.
        output->insert(std::make_pair(trackLink,
                                      DetailedTrackTruth(traj,
                                                         noiseStat,
                                                         trackStat,
                                                         noiseStat) ));
      }
    }

    // Grow sprouts from the seeds
    using SproutMap = std::map<HepMcParticleLink, Sprout>;
    SproutMap sprouts;
    while (!seeds.empty() ) {
      ATH_MSG_VERBOSE("Remaining seeds in set: "<<seeds.size());
      HepMcParticleLink link = *seeds.begin();
      Sprout current_sprout;
      std::queue<HepMC::ConstGenParticlePtr> tmp;
      ExtendedEventIndex eventIndex(link, proxy);
      HepMC::ConstGenParticlePtr current = link.cptr();
      ATH_MSG_VERBOSE("Check link: "<<link);
      const auto truthSuppressionStatus = link.getTruthSuppressionType();

    unsigned nAncestor{0};  
    do {
        HepMcParticleLink curlink( eventIndex.makeLink(HepMC::uniqueID(current), proxy));
        curlink.setTruthSuppressionType(truthSuppressionStatus);
        //// Check that the current link can be erased from the set..
        if (!nAncestor && !seeds.count(curlink)) {
           ATH_MSG_WARNING("The first link should always point to the object itself.\nHowever "<<
                            link<<"=="<<curlink<<"\n evaluates to " <<(curlink==link)<<", getTruthSuppressionType: "
                          <<(curlink.getTruthSuppressionType() == link.getTruthSuppressionType())
                          <<", id:"<<(curlink.id() == link.id())
                          <<", eventIndex: "<<(curlink.eventIndex() == link.eventIndex()));
          ATH_MSG_WARNING("Remove the link itself from the set");
          seeds.erase(link);
                            
        }
        // remove the current particle from the list of particles to consider (if it is still there)
        seeds.erase(curlink);
        
        // Have we worked on this particle before?
        SproutMap::iterator p_old = sprouts.find(curlink);
        if (p_old != sprouts.end()) {
          //merge the old sprout with the current one.
          current_sprout.splice(current_sprout.end(), p_old->second);
          current_sprout.stat += p_old->second.stat;
          // and remove the entry for the old
          sprouts.erase(p_old);
          break; // the do-while(getMother()) loop
        }
        // No, this is a new particle.  Try to extend the current truth trajectory.

        // Add the particle to the current truth trajectory.
        // New: with the current stricter cuts on mother-daughter
        // we don't have to require that ancestors produce hits.

        current_sprout.push_back(current);

        std::map<HepMcParticleLink,SubDetPRDs>::iterator p_newstat = pairStat.find(curlink);
        if (p_newstat != pairStat.end()) {
          current_sprout.stat += p_newstat->second;
        }


      } while ( (current = m_truthTrajBuilder->getMother(current)) && ++nAncestor);

      // Add the grown sprout to the list
      sprouts.insert(std::make_pair(link, current_sprout));

    } // while(!seeds.empty())

    //----------------
    // All seeds have been processed, and the upstream extensions of the
    // sprouts are done.  Extend the sprouts downstream to get the final
    // truth trajectories and store the result.
    //
    // Note: so far the "sprouts" object mapped {last particle ==> sprout}
    // Extending a sprout downstream will break this relationship,
    // but at this point we don't care about it and will only use the
    // value of the map, not the key.

    for (SproutMap::iterator s=sprouts.begin(); s!=sprouts.end(); ++s) {

      // Attempt to extend the TruthTrajectory sprout to the "outside".
      // This may add only hits that are *not* on the current track.
      // Thus no need to update stats track and stats common.

      auto current = *s->second.begin();
      while( (current = m_truthTrajBuilder->getDaughter(current)) ) {
        s->second.push_front(current);
      }

      // Now we have info to build the final TruthTrajectory.
      // FIXME: what is the current average size?
      TruthTrajectory traj;
      traj.reserve(2); // The average size is about 1.05.  Hardcode that instead of using slow list::size().
      for (Sprout::const_iterator ppart=s->second.begin(); ppart!=s->second.end(); ++ppart) {
        traj.push_back(HepMcParticleLink(ExtendedEventIndex(s->first, proxy).makeLink(HepMC::uniqueID(*ppart), proxy)));
      }

      // Count PRDs on the TruthTrajectory
      SubDetHitStatistics truthStat = countPRDsOnTruth(traj, inverseTruth);

      ATH_MSG_VERBOSE("addTrack(): sprout length = "<<traj.size());
      output->insert(std::make_pair(trackLink,
                                    DetailedTrackTruth(traj,
                                                       makeSubDetHitStatistics(s->second.stat),
                                                       trackStat,
                                                       truthStat) ));
    }


    ATH_MSG_VERBOSE("addTrack(): #sprouts = "<<sprouts.size()<<", output->size() = "<<output->size());
  }
  //================================================================
  void DetailedTrackTruthBuilder::makeTruthToRecMap( PRD_InverseTruth& result, const PRD_MultiTruthCollection& rec2truth) const {
    // invert the map from Identifier (reco hit) to HepMcParticleLink,
    // to allow lookup of all Identifiers produced by a given HepMcParticleLink.
    // the result is only used in countPRDsOnTruth. since that code ignores
    // geantinos, don't bother to sort the geantinos here.
    for ( const auto& i : rec2truth ) {
      // i.first = Identifier
      // i.second = HepMcParticleLink
      auto pa = i.second.cptr();
      if ( !pa ) {  // skip noise
        continue; 
      }
      ATH_MSG_VERBOSE("Check geantino "<<MC::isGeantino(pa)<<", is_truth_suppressed_pileup:"
                    <<HepMC::is_truth_suppressed_pileup(i.second));
      
      if ( MC::isGeantino(pa) && HepMC::is_truth_suppressed_pileup(i.second) ) { 
        continue; 
      } // skip geantinos
      result.insert(std::make_pair(i.second, i.first));
    }
  }

  //================================================================
  SubDetHitStatistics DetailedTrackTruthBuilder::countPRDsOnTruth(const TruthTrajectory& traj,
                                                                  const PRD_InverseTruth& inverseTruth) const
  {
    // Different particles from the same TruthTrajectory can contribute to the same cluster.
    // We should be careful to avoid double-counting in such cases.


    SubDetPRDs prds;
    for (TruthTrajectory::const_iterator p = traj.begin(); p != traj.end(); ++p) {
      // if this is a geantino (pile-up truth reduction), then we
      // would sum over hits from all particles that arise from this
      // pile-up collision. the result is useless. don't do it.  in
      // release 20, geantinos have pdg_id==999
      if (!(*p).cptr()) {
        ATH_MSG_WARNING( "HepMcParticleLink " << *p << " in truth trajectory does not point to a valid GenParticle.");
        continue;
      }
      if ( MC::isGeantino(*p) ) { continue; }
      using iter = PRD_InverseTruth::const_iterator;
      std::pair<iter,iter> range = inverseTruth.equal_range(*p);
      for (iter i = range.first; i != range.second; ++i) {
        SubDetHitStatistics::SubDetType subdet = findSubDetType(i->second);

        if (subdet<SubDetHitStatistics::NUM_SUBDETECTORS) prds.subDetHits[subdet].insert(i->second);
      }
    }

    return makeSubDetHitStatistics(prds);
  }

  //================================================================

} // namespace Trk
