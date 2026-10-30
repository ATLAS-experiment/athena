// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

// Local include(s):
#include "AsgAnalysisAlgorithms/TreeFillerAlg.h"

// ROOT include(s):
#include <TTree.h>

namespace CP {

   StatusCode TreeFillerAlg::initialize() {

      ANA_CHECK( m_eventInfoHandle.initialize( m_systematicsList ) );
      ANA_CHECK( m_eventSelection.initialize( m_systematicsList, m_eventInfoHandle, SG::AllowEmpty ) );
      ANA_CHECK( m_systematicsList.initialize() );

      // Return gracefully.
      return StatusCode::SUCCESS;
   }

   StatusCode TreeFillerAlg::execute(const EventContext& ctx) {
      // Skip the event if it fails the selection for every systematic
      if( m_eventSelection ) {
         bool passed = false;
         for( const auto& sys : m_systematicsList.systematicsVector() ) {
            const xAOD::EventInfo* evtInfo = nullptr;
            ANA_CHECK( m_eventInfoHandle.retrieve( evtInfo, sys, ctx ) );
            if( m_eventSelection.getBool( *evtInfo, sys ) ) {
               passed = true;
               break;
            }
         }
         if( ! passed ) {
            return StatusCode::SUCCESS;
         }
      }

      // get the output tree for the first time
      if( ! m_tree ) {
         m_tree = tree( m_treeName );
      }

      if( ! m_tree ) {
         ATH_MSG_ERROR( "Could not find output tree \"" << m_treeName
                        << "\"" );
         return StatusCode::FAILURE;
      }

      // Fill the tree.
      if( m_tree->Fill() < 0 ) {
         ATH_MSG_ERROR( "Error while filling TTree" );
         return StatusCode::FAILURE;
      }

      // Return gracefully.
      return StatusCode::SUCCESS;
   }

} // namespace CP
