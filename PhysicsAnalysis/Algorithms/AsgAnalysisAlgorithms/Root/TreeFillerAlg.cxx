// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

// Local include(s):
#include "AsgAnalysisAlgorithms/TreeFillerAlg.h"

// ROOT include(s):
#include <TTree.h>

namespace CP {

   StatusCode TreeFillerAlg::execute(const EventContext& /*ctx*/) {
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
