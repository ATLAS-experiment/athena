/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Local include(s):
#include "TrackParticleCnvAlg.h"

// EDM include(s):

#include "ParticleTruth/TrackParticleTruth.h"
#include "ParticleTruth/TrackParticleTruthKey.h"
#include "xAODTracking/TrackParticleAuxContainer.h"


#include "EventPrimitives/EventPrimitivesHelpers.h"
#include "EventPrimitives/EventPrimitivesToStringConverter.h"
#include "AthenaMonitoringKernel/Monitored.h"



namespace xAODMaker {

StatusCode
TrackParticleCnvAlg::initialize()
{

  ATH_MSG_DEBUG("Initializing TrackParticleCnvAlg");
  ATH_CHECK(m_particleCreator.retrieve());
  ATH_CHECK(m_truthClassifier.retrieve(EnableTool{m_addTruthLink}));
  ATH_CHECK(m_TrackCollectionCnvTool.retrieve());
  // to preserve the inisialised parameters of the ParticleCreatorTool:
  ATH_MSG_DEBUG("Overriding particle creator tool settings.");
  ATH_CHECK(m_TrackCollectionCnvTool->setParticleCreatorTool(&m_particleCreator));
  ATH_CHECK(m_xaodout.initialize(m_convertTracks));
  ATH_CHECK(m_tracks.initialize(m_convertTracks));
  ATH_CHECK(m_primaryVertexContainer.initialize(!m_primaryVertexContainer.key().empty()));
  ATH_CHECK(m_truthParticleLinkVec.initialize(m_addTruthLink));
  ATH_CHECK(m_trackTruth.initialize(m_addTruthLink && m_convertTracks));

  ATH_CHECK(m_truthTypeKey.initialize(m_addTruthLink));
  ATH_CHECK(m_truthOriginKey.initialize(m_addTruthLink));
  ATH_CHECK(m_truthClassKey.initialize(m_addTruthLink));
  ATH_CHECK(m_truthProbKey.initialize(m_addTruthLink));
  ATH_CHECK(m_trackLinkKey.initialize());

  // Retrieve monitoring tools if provided
  ATH_CHECK(m_trackMonitoringTool.retrieve(DisableTool{ !m_doMonitoring }));
  ATH_CHECK(m_monTool.retrieve(DisableTool{ !m_doMonitoring }));

  ATH_CHECK(m_tracksMap.initialize(m_augmentObservedTracks));

  // Return gracefully:
  return StatusCode::SUCCESS;
}

StatusCode TrackParticleCnvAlg::execute(const EventContext& ctx) const {

  const TrackCollection* tracks{};
  const xAODTruthParticleLinkVector* truthLinks{};
  const TrackTruthCollection* trackTruth{};
  const ObservedTrackMap* tracksMap{};
  const xAOD::Vertex* primaryVertex{};

  //timer object for total execution time
  auto mnt_timer_Total  = Monitored::Timer<std::chrono::milliseconds>("TIME_Total");

  // Retrieve the AOD particles:

  ATH_CHECK(SG::get(tracks, m_tracks, ctx));
  ATH_CHECK(SG::get(trackTruth, m_trackTruth, ctx));
  ATH_CHECK(SG::get(truthLinks, m_truthParticleLinkVec, ctx));
  ATH_CHECK(SG::get(tracksMap, m_tracksMap, ctx));
  // Retrieve the Tracks:
  
  if(!m_primaryVertexContainer.key().empty()) {
    const xAOD::VertexContainer* vtx_container{nullptr};
    ATH_CHECK(SG::get(vtx_container, m_primaryVertexContainer, ctx));
    const xAOD::Vertex* dummyVertex{};
    for(auto vtx : *vtx_container) {
      if(vtx->vertexType()==xAOD::VxType::PriVtx) {
        primaryVertex = vtx;
        break;
      }
      if(vtx->vertexType()==xAOD::VxType::NoVtx) {
        // in case of no primary vertex
        dummyVertex = vtx;
      }
    }
    if(!primaryVertex) {
      if(dummyVertex)
      {
        ATH_MSG_INFO("No primary vertex found, will use dummy vertex at "
                     << dummyVertex->x() << "," << dummyVertex->y() << ","
                     << dummyVertex->z());
        primaryVertex = dummyVertex;
      }
      else {
        ATH_MSG_WARNING("Neither primary nor dummy vertex found. Do Nothing.");
        return StatusCode::SUCCESS;
      }
    }
  }

  
  SG::WriteHandle wh_xaodout(m_xaodout, ctx);
  ATH_CHECK(wh_xaodout.record(std::make_unique<xAOD::TrackParticleContainer>(),
                              std::make_unique<xAOD::TrackParticleAuxContainer>()));
  ATH_CHECK(convert(ctx, (*tracks), trackTruth, *wh_xaodout, truthLinks, primaryVertex, tracksMap));

    // Monitor track parameters
    if (m_doMonitoring)
      m_trackMonitoringTool->monitor_tracks("Track", "Pass", *wh_xaodout);
  

  //extra scope needed to trigger the monitoring
  {auto monTime = Monitored::Group(m_monTool, mnt_timer_Total);}

  return StatusCode::SUCCESS;
}

StatusCode TrackParticleCnvAlg::convert(const EventContext& ctx,
                                        const TrackCollection& trackColl,
                                        const TrackTruthCollection* assocTruthColl,
                                        xAOD::TrackParticleContainer& outTrackCont,
                                        const xAODTruthParticleLinkVector* truthLinkVec,
                                        const xAOD::Vertex* primaryVertex,
                                        const ObservedTrackMap* obs_track_map) const{
  // Augment track particles using track map if available
  if (obs_track_map){
    ATH_CHECK(m_TrackCollectionCnvTool->convertAndAugment(ctx, &trackColl, &outTrackCont, obs_track_map, primaryVertex));
  } else{
    ATH_CHECK(m_TrackCollectionCnvTool->convert(ctx, &trackColl, &outTrackCont, primaryVertex));
  }
  // Create the xAOD objects:

   unsigned int trackCounter{0};
  // loop over AOD and converted xAOD for summary info and truth links
  for (xAOD::TrackParticle* particle: outTrackCont) {
    // protect if something went wrong and there is no converted xaod equivalent

    if (!particle) {
      ATH_MSG_ERROR("Failed to get an xAOD::TrackParticle");
      return StatusCode::FAILURE;
    }

    trackCounter++;
    if(msgLvl(MSG::DEBUG)){
      int npix{-1}, nsct{-1}, ntrt{-1}, npixh{-1}, nscth{-1};
      const Trk::Track *tr = particle->track();
      if (tr){
        const Trk::TrackSummary* ts = tr->trackSummary();
        if (ts) {
          npix = ts->get(Trk::numberOfPixelHits);
          nsct = ts->get(Trk::numberOfSCTHits);
          ntrt = ts->get(Trk::numberOfTRTHits);
          nscth = ts->get(Trk::numberOfSCTHoles);
          npixh = ts->get(Trk::numberOfPixelHoles);
        }
      }
      ATH_MSG_DEBUG("REGTEST: " << std::setw(5) << trackCounter
	                << "  pT:  " << std::setw(10) << particle->pt()
	                << "  eta: " << particle->eta() << "  phi: " << particle->phi()
	                << "  d0:  " << particle->d0()  << "  z0:  " << particle->z0()
	                << "\t" << npix << "/" << nsct << "/" << ntrt << "/holes/" << npixh << "/" << nscth);
    }
    //
    // --------- statistics
    //
    if (m_addTruthLink) {
      MCTruthPartClassifier::ParticleType type = MCTruthPartClassifier::Unknown;
      MCTruthPartClassifier::ParticleOrigin origin =
        MCTruthPartClassifier::NonDefined;
      unsigned int classification = 0; // Better default value here?
      float probability = -1.0;
      ElementLink<xAOD::TruthParticleContainer> link;

      ElementLink<TrackCollection> tpLink{trackColl, trackCounter -1};
      if (!tpLink.isValid()) {
        ATH_MSG_WARNING("Failed to create ElementLink to Track/TrackParticle");
      } else if(assocTruthColl->empty()){
        // This can happen if there is no HS track
        ATH_MSG_DEBUG("No truth available");
      } else {
        auto result = assocTruthColl->find(tpLink);
        if (result == assocTruthColl->end()) {
          ATH_MSG_WARNING("Failed find truth associated with Track/TrackParticle");
        } else {
          // setTruthLink(link,result->second, type, origin);
          ATH_MSG_VERBOSE("Found track Truth: uniqueID  "
                          << HepMC::uniqueID(result->second.particleLink()) << " evt "
                          << result->second.particleLink().eventIndex());
          probability = result->second.probability();
          link = truthLinkVec->find(result->second.particleLink());
          if (link.isValid()) {
            ATH_MSG_DEBUG("Found matching xAOD Truth: uniqueID "
                          << HepMC::uniqueID(*link) << " pt " << (*link)->pt()
                          << " eta " << (*link)->eta() << " phi "
                          << (*link)->phi());
            // if configured also get truth classification
            if (result->second.particleLink().cptr() &&
                !m_truthClassifier.empty()) {
              auto truthClass = m_truthClassifier->particleHepMCTruthClassifier(
                result->second.particleLink());
              type = truthClass.first;
              origin = truthClass.second;
              classification = std::get<0>(MCTruthPartClassifier::defOrigOfParticle(result->second.particleLink().cptr())); // See AGENE-2351
              ATH_MSG_VERBOSE("Got truth type  " << static_cast<int>(type)
                                                 << "  origin "
                                                 << static_cast<int>(origin)
                                                 << "  classification "
                                                 << static_cast<unsigned int>(classification));
            }
          } else {
            if (HepMC::uniqueID(result->second.particleLink()) > 0) {
              ATH_MSG_WARNING("No associated xAOD truth for valid truth link "
                              << result->second.particleLink());
            }
          }
        }
      }
      //This is the Algorithm creating TrackParticles
      //
      static const SG::AuxElement::Accessor<
        ElementLink<xAOD::TruthParticleContainer>>
        theLink("truthParticleLink");
      static const SG::AuxElement::Accessor<float> theProbability(
        "truthMatchProbability");
      theLink(*particle) = link;
      theProbability(*particle) = probability;
      if (!m_truthClassifier.empty()) {
        static const SG::AuxElement::Accessor<int> theType("truthType");
        static const SG::AuxElement::Accessor<int> theOrigin("truthOrigin");
        static const SG::AuxElement::Accessor<unsigned int> theClassification("truthClassification");
        theType(*particle) = static_cast<int>(type);
        theOrigin(*particle) = static_cast<int>(origin);
        theClassification(*particle) = static_cast<unsigned int>(classification);
      }
    }
  } // loop over aod tracks

  ATH_MSG_DEBUG("Converted [" << trackColl.size() << " -> " << outTrackCont.size()
                              << "] TrackParticles");
  if (trackColl.size() != outTrackCont.size()) {
    ATH_MSG_ERROR("number of items in the AOD container: "
                    << trackColl.size()
                    << " is not equal to the number of items in its converted "
                       "xAOD equivalent: "
                    << outTrackCont.size());
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}
} // namespace xAODMaker
