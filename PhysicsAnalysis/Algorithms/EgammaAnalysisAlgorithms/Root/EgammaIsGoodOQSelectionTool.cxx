//
// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
//

// Local include(s):
#include "EgammaAnalysisAlgorithms/EgammaIsGoodOQSelectionTool.h"


// System include(s):
#include <iomanip>

namespace CP {

   const asg::AcceptInfo& EgammaIsGoodOQSelectionTool::getAcceptInfo() const {

      // Return the internal object.
      return m_accept;
   }

  asg::AcceptData EgammaIsGoodOQSelectionTool::
   accept( const xAOD::IParticle* part ) const {

      // Reset the decision object.
      asg::AcceptData accept {&m_accept};

      // Cast the particle to an e/gamma type.
      const xAOD::Egamma* eg = nullptr;
      if( ( part->type() != xAOD::Type::Electron ) &&
          ( part->type() != xAOD::Type::Photon ) ) {
         ATH_MSG_WARNING( "Non-e/gamma object received" );
         return accept;
      }
      eg = static_cast< const xAOD::Egamma* >( part );

      // Calculate the decision.
      accept.setCutResult( m_oqCutIndex, eg->isGoodOQ( m_mask ) );

      // Cut based on the dead HV Tool removal
      accept.setCutResult(m_deadHVCutIndex, m_deadHVTool->accept(eg));

      // Return the internal object.
      return accept;
   }

   StatusCode EgammaIsGoodOQSelectionTool::initialize() {

      // Tell the user what is going to happen.
      ATH_MSG_INFO( "Selecting e/gamma objects with OQ mask: 0x"
                    << std::hex << m_mask );

      // Set up the TAccept object.
      m_oqCutIndex = m_accept.addCut( "EgammaOQ", "Egamma object quality cut" );
      m_deadHVCutIndex = m_accept.addCut("notDeadHV", "Egamma dead HV removal cut");

      // Set up the dead HV Removal Tool
      m_deadHVTool.setTypeAndName("AsgDeadHVCellRemovalTool/deadHVTool");
      if (m_deadHVTool.retrieve().isFailure()){
         ANA_MSG_ERROR("Failed to retrieve DeadHVTool, aborting");
         return StatusCode::FAILURE;
      }


      // Return gracefully.
      return StatusCode::SUCCESS;
   }

} // namespace CP
