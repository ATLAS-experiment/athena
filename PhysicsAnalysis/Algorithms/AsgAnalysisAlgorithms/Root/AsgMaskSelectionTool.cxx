/*
 Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 */

/// @author RD Schaffer



//
// includes
//

#include <AsgAnalysisAlgorithms/AsgMaskSelectionTool.h>

#include <cmath>

//
// method implementations
//

namespace CP
{

    StatusCode AsgMaskSelectionTool ::
    initialize ()
    {
      if(m_selVars.size() != m_selMasks.size()) {
          ATH_MSG_ERROR("Property selectionMasks has different size to selectionVars. Please check your configuration");
          return StatusCode::FAILURE;
      }
      // Could also warn if there are fewer values, but we don't have to force users to set where irrelevant.
      // Maybe warn unless the size is 0, in which case assume all default?

      for(size_t index=0; index<m_selVars.size(); ++index) {
          const std::string& thisVar = m_selVars[index];
          if (thisVar.empty()) {
              ATH_MSG_ERROR("Empty string passed as selection variable!");
              return StatusCode::FAILURE;
          } else {
              // Setup acceptInfo and create accessor to read variable on execute
              m_accept.addCut (thisVar, thisVar);
              std::unique_ptr<ISelectionReadAccessor> accessor;
              ATH_CHECK (makeSelectionReadAccessor (thisVar, accessor));
              m_acc_selVars.push_back (std::move (accessor));
          }
      }

      return StatusCode::SUCCESS;
    }

    const asg::AcceptInfo& AsgMaskSelectionTool ::
    getAcceptInfo () const
    {
      return m_accept;
    }

    asg::AcceptData AsgMaskSelectionTool ::
    accept (const xAOD::IParticle *particle) const
    {
      asg::AcceptData accept (&m_accept);
      for(std::size_t cutIndex=0; cutIndex<m_accept.getNCuts(); ++cutIndex) {
          // Apply mask and test
          int mask = m_selMasks[cutIndex];
          ATH_MSG_VERBOSE("Now testing var \"" << m_selVars[cutIndex] << "\" requiring value " << mask);
          accept.setCutResult (cutIndex, ((m_acc_selVars[cutIndex]->getBits (*particle) & mask ) == 0 ));
      }

      return accept;
    }
}

