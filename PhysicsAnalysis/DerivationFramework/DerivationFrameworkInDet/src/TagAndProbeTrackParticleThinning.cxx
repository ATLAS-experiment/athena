/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// TagAndProbeTrackParticleThinning.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#include "DerivationFrameworkInDet/TagAndProbeTrackParticleThinning.h"
#include "xAODTracking/TrackParticlexAODHelpers.h"
#include "StoreGate/ThinningHandle.h"

// Constructor
DerivationFramework::TagAndProbeTrackParticleThinning::TagAndProbeTrackParticleThinning(const std::string& t,
											const std::string& n,
											const IInterface* p ) :
  base_class(t, n, p)
{}

// Athena initialize and finalize
StatusCode DerivationFramework::TagAndProbeTrackParticleThinning::initialize()
{
  //check xAOD::InDetTrackParticle collection
  ATH_CHECK( m_inDetParticlesKey.initialize (m_streamName) );
  ATH_MSG_DEBUG("Using " << m_inDetParticlesKey << "as the source collection for inner detector track particles");
  ATH_CHECK( m_vertexContainerKey.initialize() );
  ATH_CHECK( m_muonContainerKey.initialize() );

  if (!m_selectionString.empty()) {
    ATH_CHECK(initializeParser(m_selectionString));
  }

  return StatusCode::SUCCESS;
}

StatusCode DerivationFramework::TagAndProbeTrackParticleThinning::finalize()
{
  ATH_MSG_INFO("Processed "<< m_ntot <<" tracks, "<< m_npass<< " were retained ");
  ATH_CHECK( finalizeParser() );
  return StatusCode::SUCCESS;
}

// The thinning itself
StatusCode DerivationFramework::TagAndProbeTrackParticleThinning::doThinning() const
{

  const EventContext& ctx = Gaudi::Hive::currentContext();

  // Retrieve main TrackParticle collection
  SG::ThinningHandle<xAOD::TrackParticleContainer> importedTrackParticles
    (m_inDetParticlesKey, ctx);

  // Check the event contains tracks
  size_t nTracks = importedTrackParticles->size();
  if (nTracks==0) return StatusCode::SUCCESS;

  // Set up a mask with the same entries as the full TrackParticle collection
  std::vector<bool> mask;
  mask.assign(nTracks,false); // default: don't keep any tracks

  std::lock_guard<std::mutex> lock(m_mutex);    

  m_ntot += nTracks;

  // Execute the text parser and update the mask
  std::vector<int> entries;
  if (m_parser) {
    entries =  m_parser->evaluateAsVector();
    unsigned int nEntries = entries.size();
    // check the sizes are compatible
    if (nTracks != nEntries ) {
      ATH_MSG_ERROR("Sizes incompatible! Are you sure your selection string used ID TrackParticles?");
      return StatusCode::FAILURE;
    } else {
      // set mask
      for (unsigned int i=0; i<nTracks; ++i) if (entries[i]==1) mask[i]=true;
    }
  }

  // Retrieve vertex collection
  SG::ReadHandle<xAOD::VertexContainer> primaryVertices(m_vertexContainerKey,ctx);
  ATH_CHECK( primaryVertices.isValid() );
  
  const xAOD::Vertex* priVtx = nullptr;
  
  for( const auto* vtx : *primaryVertices ) {
    if( vtx->vertexType() == xAOD::VxType::PriVtx ) {
      priVtx = vtx;
      break;
    }
  }

  if( !priVtx ) return StatusCode::SUCCESS;
  
  // Retrieve muon collection
  SG::ReadHandle<xAOD::MuonContainer> muons(m_muonContainerKey,ctx);
  ATH_CHECK( muons.isValid() );

  // Count the mask
  for (size_t i=0; i<nTracks; ++i) {
    
    const auto* trk = importedTrackParticles->at(i);

    // Keep only d0 significancd is small enough
    if( std::abs( xAOD::TrackingHelpers::d0significance( trk ) ) > m_d0SignifCut ) {
      continue;
    }
    
    // Keep only delta(z0) is less than 10 mm
    if( std::abs( trk->z0() - priVtx->z() + trk->vz() ) > m_z0Cut ) {
      continue;
    }

    // probe track keeping
    for (const auto muon : *muons) {
      const auto* muon_trk = muon->trackParticle(xAOD::Muon::InnerDetectorTrackParticle);

      if( muon_trk ) {
	if( muon_trk == trk) continue;
      }

      if( (muon->p4() + trk->p4()).M() > m_massCut ) {
	mask.at(i) = (mask.at(i) && true);
	++m_npass;
      }
    }
        
  }

  assert( importedTrackParticles->size() == mask.size() );

  // Execute the thinning service based on the mask.
  importedTrackParticles.keep (mask);
  
  return StatusCode::SUCCESS;
}

