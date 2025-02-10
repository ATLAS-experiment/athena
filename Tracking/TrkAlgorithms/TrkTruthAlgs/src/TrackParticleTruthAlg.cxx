/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TrackParticleTruthAlg.h"

#include "TrkTruthData/TrackTruthKey.h"
#include "xAODCore/AuxStoreAccessorMacros.h"
#include "GeneratorObjects/McEventCollection.h"
#include "StoreGate/WriteDecorHandle.h"
#include "TruthUtils/MagicNumbers.h"


// -----------------------------------------------------------------------------------------------------
StatusCode TrackParticleTruthAlg::initialize() {

  ATH_CHECK(m_trkKey.initialize());
  ATH_CHECK(m_particlesLinkKey.initialize());
  ATH_CHECK(m_particlesTypeKey.initialize());
  ATH_CHECK(m_particlesOriginKey.initialize());
  ATH_CHECK(m_truthParticleLinkVecKey.initialize());
  ATH_CHECK(m_truthTracksKey.initialize());
  ATH_CHECK(m_truthClassifier.retrieve());
  return StatusCode::SUCCESS;
}


// -----------------------------------------------------------------------------------------------------
StatusCode TrackParticleTruthAlg::execute(const EventContext& ctx) const {

  SG::ReadHandle<TrackTruthCollection> truthTracks{m_truthTracksKey, ctx};
  SG::ReadHandle<xAODTruthParticleLinkVector> truthParticleLinkVec{ m_truthParticleLinkVecKey, ctx};
  SG::WriteDecorHandle<xAOD::TrackParticleContainer,ElementLink<xAOD::TruthParticleContainer> > particlesLink{m_particlesLinkKey, ctx};
  SG::WriteDecorHandle<xAOD::TrackParticleContainer,int> particlesType{m_particlesTypeKey, ctx};
  SG::WriteDecorHandle<xAOD::TrackParticleContainer,int> particlesOrigin{m_particlesOriginKey, ctx};

  ATH_CHECK(particlesLink.isValid());
  ATH_CHECK(truthParticleLinkVec.isValid());
  ATH_CHECK(truthTracks.isValid());


  const TrackTruthCollection truthTrackColl=*truthTracks;
  const xAODTruthParticleLinkVector truthParticleLinks=*truthParticleLinkVec;

  int partInd=0;
  for( const xAOD::TrackParticle* particle : *particlesLink ){

    ATH_MSG_DEBUG("Looking up truth for pt " << particle->pt() << " eta " << particle->eta() << " phi " << particle->phi());
    /// STACO combined tracks are just added to the muon for completeness
    /// They're not expected to have a valid track link
    if( !particle->trackLink().isValid()){
      if (!particle->patternRecoInfo()[xAOD::STACO])  {
          ATH_MSG_WARNING("Found TrackParticle with Invalid element link, skipping");
      }
      //add dummy truth link
      particlesLink(*particle)=ElementLink<xAOD::TruthParticleContainer>();
      particlesType(*particle) = 0;
      particlesOrigin(*particle)= 0;
      continue;
    }

    MCTruthPartClassifier::ParticleType type = MCTruthPartClassifier::Unknown;
    MCTruthPartClassifier::ParticleOrigin origin = MCTruthPartClassifier::NonDefined;
    ElementLink<xAOD::TruthParticleContainer> link;
    // look-up associated truth particle

    Trk::TrackTruthKey key(particle->trackLink());
    auto result = truthTrackColl.find(key);
    
    // if we found a match use it
    if( result != truthTrackColl.end() ){
      ATH_MSG_VERBOSE("Found track Truth: uniqueID  " << HepMC::uniqueID(&(result->second.particleLink())) << " evt " << result->second.particleLink().eventIndex());
      link = truthParticleLinks.find(result->second.particleLink());
      
      // if configured also get truth classification

      if( link.isValid()&& !m_truthClassifier.empty() ){
	auto truthClass = m_truthClassifier->particleTruthClassifier(*link);
	type = truthClass.first;
	origin = truthClass.second;
	ATH_MSG_VERBOSE("Got truth type  " << static_cast<int>(type) << "  origin " << static_cast<int>(origin));
      }
    }

    if( link.isValid() ){
      ATH_MSG_DEBUG("Found matching xAOD Truth: uniqueID " << HepMC::uniqueID(*link) << " pt " << (*link)->pt() << " eta " << (*link)->eta() << " phi " << (*link)->phi());
      // set element link 
      link.toPersistent();
      particlesLink(*particle)=link;

    }
    else{ //no truth link, add a dummy
      particlesLink(*particle)=ElementLink<xAOD::TruthParticleContainer>();
    }

    if( !m_truthClassifier.empty() ){
      //use index for these since it's the same particle
      particlesType(partInd)=static_cast<int>(type);
      particlesOrigin(partInd)=static_cast<int>(origin);
    }
    partInd++;
  }
  return StatusCode::SUCCESS;
}
