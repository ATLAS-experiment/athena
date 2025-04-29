/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "DerivationFrameworkTau/TauThinningTool.h"
#include "StoreGate/ThinningHandle.h"
#include "GaudiKernel/ThreadLocalContext.h"
#include <vector>
#include <string>


DerivationFramework::TauThinningTool::TauThinningTool(const std::string& t,
						      const std::string& n,
						      const IInterface* p) :
  base_class(t,n,p)
{
}


StatusCode DerivationFramework::TauThinningTool::initialize()
{
  ATH_CHECK( m_taus.initialize(m_streamName) );
  ATH_CHECK( m_tauTracks.initialize(m_streamName) );
  ATH_CHECK( m_trackParticles.initialize(m_streamName) );
  ATH_CHECK( m_neutralPFOs.initialize(m_streamName) );
  ATH_CHECK( m_secondaryVertices.initialize(m_streamName) );

  // set up the text-parsing machinery for selecting taus according to user cuts
  if (!m_selectionString.empty()) {
    ATH_MSG_INFO("Selection string for " << m_taus.key() << ": " << m_selectionString);
    ATH_CHECK( initializeParser(m_selectionString) );
  }
  return StatusCode::SUCCESS;
}


StatusCode DerivationFramework::TauThinningTool::finalize()
{
  ATH_MSG_INFO("Processed " << m_ntot << " taus, " << m_npass << " were kept");
  ATH_CHECK( finalizeParser() );
  return StatusCode::SUCCESS;
}


StatusCode DerivationFramework::TauThinningTool::doThinning() const
{
  const EventContext& ctx = Gaudi::Hive::currentContext();

  // retrieve containers and thin them
  SG::ThinningHandle<xAOD::TauJetContainer> taus(m_taus, ctx);
  taus.thinAll();
  size_t nTaus = taus->size();

  SG::ThinningHandle<xAOD::TauTrackContainer> tauTracks(m_tauTracks, ctx);
  tauTracks.thinAll();

  SG::ThinningHandle<xAOD::TrackParticleContainer> trackParticles(m_trackParticles, ctx);
  trackParticles.thinAll();

  SG::ThinningHandle<xAOD::PFOContainer> neutralPFOs(m_neutralPFOs, ctx);
  neutralPFOs.thinAll();

  SG::ThinningHandle<xAOD::VertexContainer> secondaryVertices(m_secondaryVertices, ctx);
  secondaryVertices.thinAll();


  std::vector<const xAOD::TauJet*> tausToKeep;

  // execute the text parser if requested
  if (!m_selectionString.empty()) {
    std::vector<int> entries =  m_parser->evaluateAsVector();
    size_t nEntries = entries.size();
    if (nTaus != nEntries) {
      ATH_MSG_ERROR("Incompatible sizes: " << nTaus << " vs " << nEntries << "! Please check your selection string uses the appropriate tau container.");
      return StatusCode::FAILURE;
    }
    // identify which taus to keep
    for (size_t i=0; i<nTaus; ++i) if (entries[i]==1) tausToKeep.push_back(taus->at(i));    
  }
  // use all taus if no selection string is passed
  else {
    for (size_t i=0; i<nTaus; ++i) tausToKeep.push_back(taus->at(i));
  }

  // protection against duplicate taus -- built from different seed jets, but end up having same (eta,phi)
  if( tausToKeep.size() > 0){
    for(size_t i=0; i < tausToKeep.size()-1; i++){
      const auto* aTau=tausToKeep[i];
      auto it = std::remove_if(tausToKeep.begin()+i+1,tausToKeep.end(),[aTau](const xAOD::TauJet* bTau) {return aTau->p4().DeltaR(bTau->p4()) < 0.01;});
      tausToKeep.erase (it, tausToKeep.end());
    }
  }   

  // keep the various tau-related objects for taus passing the selection
  for (const auto* tau : tausToKeep) {
    // tau
    taus.keep(tau->index());

    // classifiedCharged tau tracks
    for (const xAOD::TauTrack* track : tau->tracks()) {
      tauTracks.keep(track->index());
      
      // associated ID track
      trackParticles.keep(track->track()->index());
    }

    // neutral PFOs
    for (size_t i=0; i<tau->nNeutralPFOs(); i++) {
      neutralPFOs.keep(tau->neutralPFO(i)->index());
    }  

    // secondary vertex
    if (tau->secondaryVertex() != nullptr) {
      secondaryVertices.keep(tau->secondaryVertex()->index());
    }
  }

  // increment counters
  m_npass += tausToKeep.size();
  m_ntot  += nTaus;

  return StatusCode::SUCCESS;
}
