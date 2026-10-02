//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//

// Local include(s):
#include "EgammaAnalysisAlgorithms/EgammaIsGoodOQSelectionTool.h"

#include "AsgTools/AsgToolConfig.h"
#include <xAODEventInfo/EventInfo.h>
#include <AsgDataHandles/ReadDecorHandleKey.h>

namespace CP {

   EgammaIsGoodOQSelectionTool::EgammaIsGoodOQSelectionTool( const std::string& name )
      : asg::AsgTool( name ) {
   }

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
                    << std::hex << m_mask << std::dec );

      // Set up the TAccept object.
      m_oqCutIndex = m_accept.addCut( "EgammaOQ", "Egamma object quality cut" );
      m_deadHVCutIndex = m_accept.addCut("notDeadHV", "Egamma dead HV removal cut");

      // Set up the dead HV Removal Tool
      if (m_deadHVTool.empty())
      {
         asg::AsgToolConfig config("AsgDeadHVCellRemovalTool/deadHVTool");
         ANA_CHECK(config.makePrivateTool(m_deadHVTool));
      }
      if (m_deadHVTool.retrieve().isFailure()){
         ANA_MSG_ERROR("Failed to retrieve DeadHVTool, aborting");
         return StatusCode::FAILURE;
      }

#ifndef XAOD_STANDALONE
      // Declare dependency on RandomRunNumber for MC (used by deadHVTool)
      // Not doing this in the tool itself, since it is a public tool and
      // for public tools the dependencies don't get properly propagated
      SG::ReadDecorHandleKey<xAOD::EventInfo> randomRunNumberKey{ "EventInfo.RandomRunNumber" };
      addDependency(randomRunNumberKey.fullKey(), Gaudi::DataHandle::Reader);
#endif

      // Return gracefully.
      return StatusCode::SUCCESS;
   }

} // namespace CP
