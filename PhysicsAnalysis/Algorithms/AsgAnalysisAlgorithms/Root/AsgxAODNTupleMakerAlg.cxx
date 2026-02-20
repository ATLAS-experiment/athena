// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

// Local include(s):
#include "AsgAnalysisAlgorithms/AsgxAODNTupleMakerAlg.h"

// EDM include(s):
#include "AthContainersInterfaces/IAuxStoreIO.h"
#include "AthContainersInterfaces/IAuxTypeVectorFactory.h"
#include "AthContainers/AuxElement.h"
#include "AthContainers/AuxVectorBase.h"
#include "AthContainers/normalizedTypeinfoName.h"
#include "xAODRootAccess/tools/THolder.h"

// ROOT include(s):
#include <TClass.h>
#include <TTree.h>
#include <TBranch.h>
#include <TVirtualCollectionProxy.h>

// System include(s):
#include <regex>
#include <algorithm>
#include <functional>
#include <sstream>


namespace CP {

   StatusCode AsgxAODNTupleMakerAlg::initialize() {

      // Check that at least one branch is configured.
      if( m_branches.empty() ) {
         ATH_MSG_ERROR( "No branches set up for writing" );
         return StatusCode::FAILURE;
      }

      // Set up the systematics list.
      ATH_CHECK( m_systematicsService.retrieve() );

      // Initialise the output tree.
      m_tree = tree( m_treeName );
      if( ! m_tree ) {
         ATH_MSG_ERROR( "Could not find output tree \"" << m_treeName
                        << "\"" );
         return StatusCode::FAILURE;
      }
      // Call the setup function.
      if (m_defaultBasketSize != 0)
         m_processorList.defaultBasketSize = m_defaultBasketSize;
      ATH_CHECK( m_processorList.setupTree (m_branches, {m_nonContainers.value().begin(), m_nonContainers.value().end()}, *m_systematicsService, *m_tree) );

      // Return gracefully.
      return StatusCode::SUCCESS;
   }

   StatusCode AsgxAODNTupleMakerAlg::execute() {

      ATH_CHECK( m_processorList.process (*evtStore()) );

      // Return gracefully.
      return StatusCode::SUCCESS;
   }

   StatusCode AsgxAODNTupleMakerAlg::finalize() {

      // Return gracefully.
      return StatusCode::SUCCESS;
   }

} // namespace CP
