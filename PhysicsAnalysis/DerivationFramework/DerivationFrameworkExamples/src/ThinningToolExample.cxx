/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// ThinningToolExample.cxx
///////////////////////////////////////////////////////////////////
// Author: James Catmore (James.Catmore@cern.ch)
// This is a trivial example of an implementation of a thinning tool 
// which removes all ID tracks which do not pass a user-defined cut.

#include "ThinningToolExample.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "StoreGate/ThinningHandle.h"
#include "GaudiKernel/ThreadLocalContext.h"
#include <vector>
#include <string>


// Destructor
DerivationFramework::ThinningToolExample::~ThinningToolExample() {
}  


// Athena initialize and finalize
StatusCode DerivationFramework::ThinningToolExample::initialize()
{
     ATH_CHECK( m_inDetSGKey.initialize (m_streamName) );
     return StatusCode::SUCCESS;
}


StatusCode DerivationFramework::ThinningToolExample::finalize()
{
     ATH_MSG_INFO("Processed "<< m_ntot <<" tracks, "<< m_npass<< " were retained ");
     return StatusCode::SUCCESS;
}


// The thinning itself
StatusCode DerivationFramework::ThinningToolExample::doThinning() const
{
      const EventContext& ctx = Gaudi::Hive::currentContext();

      // Get the track container
      SG::ThinningHandle<xAOD::TrackParticleContainer> tracks (m_inDetSGKey, ctx);
      m_ntot += tracks->size();
      // Loop over tracks, see if they pass, set mask
      std::vector<bool> mask;
      for (const xAOD::TrackParticle* track : *tracks) {
         if ( track->pt() > m_trackPtCut ) {++m_npass; mask.push_back(true);}
         else { mask.push_back(false); }
     }
     tracks.keep (mask);

     return StatusCode::SUCCESS;
}  
  
