/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//==================================================
// Selection of D*+ -> pi+ + D0
//==================================================

#include "DerivationFrameworkSM/DStarSelectionTool.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "StoreGate/ThinningHandle.h"
#include "GaudiKernel/ThreadLocalContext.h"
#include "Gaudi/Property.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteDecorHandle.h"

namespace{
  static const SG::AuxElement::ConstAccessor<Char_t> flag_D0("passed_D0");
  static const SG::AuxElement::ConstAccessor<Char_t> flag_D0b("passed_D0b"); 
  static const SG::Accessor<std::vector<float>> acc_reFit_Px("RefTrackPx");
  static const SG::Accessor<std::vector<float>> acc_reFit_Py("RefTrackPy");
  static const SG::Accessor<std::vector<float>> acc_reFit_Pz("RefTrackPz");
}


namespace DerivationFramework {
  
StatusCode DStarSelectionTool::initialize() {
    ATH_MSG_DEBUG("in initialize()");
    ATH_CHECK(m_trackKey.initialize());
    ATH_CHECK(m_inputVtxContainerName.initialize());    
    ATH_CHECK(m_trackDecoKey.initialize());
    ATH_CHECK(m_vertexDecoKey.initialize());    
    return StatusCode::SUCCESS;
    
}

StatusCode DStarSelectionTool::addBranches(const EventContext& ctx) const {
  
  // Track container
  const xAOD::TrackParticleContainer* trackParticleContainer{nullptr};
  ATH_CHECK(SG::get(trackParticleContainer, m_trackKey, ctx));    
  
  // Vertex container
  const xAOD::VertexContainer* D0Container{nullptr};
  ATH_CHECK(SG::get(D0Container, m_inputVtxContainerName, ctx));
  
  // Mask for track / vertex skimming
  std::vector<char> track_pass_map(trackParticleContainer->size());
  std::vector<char> vertex_pass_map(D0Container->size());
  
  // Loop over D0 candidate vertices
  for(const xAOD::Vertex* vertex: *D0Container) {
        
    // Check vertex passes loose D0 cuts defined in python 
    
    bool passed_D0_or_D0b = (flag_D0(*vertex) || flag_D0b(*vertex));
    
    // Only consider vertices which have passed D0 mass window for 
    // matching to soft pion canddiates
    if(!passed_D0_or_D0b) continue;
    
    const xAOD::TrackParticle* track1 = vertex->trackParticle(0);
    const xAOD::TrackParticle* track2 = vertex->trackParticle(1);

    if(!track1) { ATH_MSG_WARNING("Could not find track at D0 vertex (index 0)"); continue; }
    if(!track2) { ATH_MSG_WARNING("Could not find track at D0 vertex (index 1)"); continue; }
    
        // Re-fitted D0 track 4-vectors
    
    const std::vector<float>& reFit_Px = acc_reFit_Px(*vertex);
    const std::vector<float>& reFit_Py = acc_reFit_Py(*vertex);
    const std::vector<float>& reFit_Pz = acc_reFit_Pz(*vertex);
    
    TLorentzVector track1_pion, track1_kaon;
    track1_pion.SetXYZM(reFit_Px.at(0),reFit_Py.at(0),reFit_Pz.at(0),m_pionMass);
    track1_kaon.SetXYZM(reFit_Px.at(0),reFit_Py.at(0),reFit_Pz.at(0),m_kaonMass);
    
    TLorentzVector track2_pion, track2_kaon;
    track2_pion.SetXYZM(reFit_Px.at(1),reFit_Py.at(1),reFit_Pz.at(1),m_pionMass);
    track2_kaon.SetXYZM(reFit_Px.at(1),reFit_Py.at(1),reFit_Pz.at(1),m_kaonMass);
    
    // Consider both D0 and D0bar hypotheses (irrespective of charges)
    TLorentzVector D0_hypo1 = track1_pion + track2_kaon;
    TLorentzVector D0_hypo2 = track1_kaon + track2_pion;
    
    bool passed_Dstar = false;
	
    // Loop over tracks
    for(const xAOD::TrackParticle* track3: *trackParticleContainer) {
      
      // Avoid duplicates
      if( track1->index() == track3->index() ) continue;
      if( track2->index() == track3->index() ) continue;
      
      TLorentzVector track3_pion= track3->p4();
	    
            const double deltaM_hypo1 = (D0_hypo1 + track3_pion).M() - D0_hypo1.M();
            const double deltaM_hypo2 = (D0_hypo2 + track3_pion).M() - D0_hypo2.M();

            // Selection of loose D*+ ->pi+ + D0 candidates based on mass difference
            // Keep both "right" and "wrong" charge combinations
            if( deltaM_hypo1 < m_deltaMassMax || deltaM_hypo2 < m_deltaMassMax ) {

                passed_Dstar = true;

                // Mark all tracks as being part of a D*+ -> D0 + pi+ (+c.c.) candidate
                track_pass_map[track1->index()] = true;
                track_pass_map[track2->index()] = true;
                track_pass_map[track3->index()] = true;

            }

        } // Loop over tracks

        // Adjust mask if we'd like to retain this vertex
        if(passed_Dstar) {
            vertex_pass_map[vertex->index()] = true;
        }

  } // Loop over D0 candidate vertices
  
  // Decorate tracks with skim decision flag
  
  SG::WriteDecorHandle<xAOD::TrackParticleContainer, int> flagTrackPass(m_trackDecoKey,ctx);
  // Loop over tracks
  for(const xAOD::TrackParticle* track:*trackParticleContainer) {
    
    flagTrackPass(*track)  = track_pass_map[track->index()] ; // Mark track to keep/drop during skimming 
    
  } // Loop over tracks

  // Decorate vertex for skimming decision (Char used for compatability for BPhys skim/slim tools)
  SG::WriteDecorHandle<xAOD::VertexContainer, Char_t> flagVertexPass(m_vertexDecoKey,ctx);
  
  // Loop over D0 candidate vertices
  for(const xAOD::Vertex* vertex: *D0Container) {
    
    // Mark vertex to keep during skimming, based on earlier decision
    flagVertexPass(*vertex) = vertex_pass_map[vertex->index()];
    
  } // Loop over D0 candidate vertices
  
  return StatusCode::SUCCESS;
}  
  
  
  
}
