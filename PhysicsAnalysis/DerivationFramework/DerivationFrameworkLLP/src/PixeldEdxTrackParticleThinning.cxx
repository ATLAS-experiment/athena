/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// PixeldEdxTrackParticleThinning.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#include "DerivationFrameworkLLP/PixeldEdxTrackParticleThinning.h"
#include "xAODTracking/TrackStateValidationContainer.h"
#include "xAODTracking/TrackMeasurementValidationContainer.h"
#include "xAODTracking/TrackParticlexAODHelpers.h"
#include "StoreGate/ThinningHandle.h"
#include <vector>
#include <string>

const std::vector<double> DerivationFramework::PixeldEdxTrackParticleThinning::m_preScales = {
  150, 150, 150, 150, 150, 150, 150, 150, 150, 150,
  150, 150, 150, 150, 150, 150, 150, 150, 150, 150,
  150, 150, 150, 150, 150, 124.261, 111.448, 97.7357, 85.5616, 72.3294,
  61.0017, 50.8592, 41.6827, 33.7413, 26.9884, 21.3822, 16.8352, 13.1081, 10.2368, 7.96723,
  6.23636, 4.91757, 3.91519, 3.15519, 2.57768, 2.13622, 1.80524, 1.53102, 1.30677, 1.13582
};

// Constructor
DerivationFramework::PixeldEdxTrackParticleThinning::PixeldEdxTrackParticleThinning(const std::string& t,
                                                                                    const std::string& n,
                                                                                    const IInterface* p ) :
  base_class(t, n, p)
{}

// Athena initialize and finalize
StatusCode DerivationFramework::PixeldEdxTrackParticleThinning::initialize()
{
  ATH_MSG_DEBUG("initialize() ...");
    
  //check xAOD::InDetTrackParticle collection
  ATH_CHECK( m_inDetParticlesKey.initialize (m_streamName) );
  ATH_MSG_INFO("Using " << m_inDetParticlesKey << "as the source collection for inner detector track particles");
  ATH_CHECK( m_vertexContainerKey.initialize() );
  //check availability of xAOD::TrackStateValidation and xAOD::TrackMeasurementValidation containers

  if (!m_selectionString.empty()) {
    ATH_CHECK(initializeParser(m_selectionString));
  }

  m_counter.resize(50);
  m_counter_picked.resize(50);
  //////////////////////////////////////////////////////////////////////////////////////////
  
  if( 0 == m_pTbins.size() ) {
    for( size_t ip = 0; ip <= 50; ++ip ) {
      m_pTbins.emplace_back( pow( 10, 2.0 + 0.04 * ip ) );
    }
  }

  //////////////////////////////////////////////////////////////////////////////////////////

  return StatusCode::SUCCESS;
}

StatusCode DerivationFramework::PixeldEdxTrackParticleThinning::finalize()
{
  ATH_MSG_DEBUG("finalize() ...");
  ATH_MSG_INFO("Processed "<< m_ntot <<" tracks, "<< m_npass<< " were retained ");
  ATH_CHECK( finalizeParser() );
  return StatusCode::SUCCESS;
}

// The thinning itself
StatusCode DerivationFramework::PixeldEdxTrackParticleThinning::doThinning() const
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
  if (m_parser) {
    std::vector<int> entries =  m_parser->evaluateAsVector();
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

  auto getBin = [&]( const double pT ) -> size_t {
    for( size_t ip = 0; ip < m_pTbins.size()-1; ip++ ) {
      if( pT > m_pTbins.at(ip) && pT < m_pTbins.at(ip+1) ) {
        return ip;
      }
    }
    return 0;
  };
    
  // Count the mask
  for (size_t i=0; i<nTracks; ++i) {
    
    const auto* trk = importedTrackParticles->at(i);
    
    // Keep all tracks if IBL is overflow
    if( trk->numberOfIBLOverflowsdEdx() > 0 ) {
      
      mask.at(i) = true; ++m_npass; continue;
      
    }
    
    
    // Keep all track for pT above 10 GeV
    if( std::abs( trk->pt() ) > m_unprescalePtCut ) {
      
      mask.at(i) = true; ++m_npass; continue;
      
    }      
    
    // Hereafter only low-pT and no-overflow tracks
    
    // Keep only d0 significancd is small enough
    if( std::abs( xAOD::TrackingHelpers::d0significance( trk ) ) > m_d0SignifCut ) {
      continue;
    }
    
    // Keep only delta(z0) is less than 10 mm
    if( std::abs( trk->z0() - priVtx->z() + trk->vz() ) > m_z0Cut ) {
      continue;
    }
    
    // Reject forward low-pT tracks
    if( std::abs( trk->eta() ) > m_etaCut ) {
      continue;
    }
    
    // Prescaled track keeping
    auto bin = getBin( std::abs( trk->pt() ) );
    
    const auto preScale   = static_cast<unsigned long long>( std::floor( m_preScales.at( bin ) * m_globalScale ) );
    const auto preScale10 = std::max( static_cast<unsigned long long>( std::floor( m_preScales.at( bin ) * m_globalScale / 10. ) ), 1ull );
        
    float dEdx { 0 };
    trk->summaryValue( dEdx, xAOD::pixeldEdx );

    // Increment the m_counter
    m_counter.at(bin)++;

    // Relatively higher dE/dx tracks
    static const float dEdxThr { static_cast<float>(pow( 10, 0.1 )) };
    if( dEdx > dEdxThr ) {
      
      mask.at(i) = ( (m_counter.at(bin) % preScale10) == 0 );
      
      // There are also tracks with dE/dx == -1.
    } else if( dEdx > 0. ) {
      
      mask.at(i) = ( (m_counter.at(bin) % preScale) == 0 );
      
    } else {
      
      mask.at(i) = false;
      
    }
    
    if( mask.back() ) { ++m_npass; m_counter_picked.at(bin)++; }
    
  }

  assert( importedTrackParticles->size() == mask.size() );

  // Execute the thinning service based on the mask.
  importedTrackParticles.keep (mask);
  
  return StatusCode::SUCCESS;
}

