/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include <AsgAnalysisAlgorithms/AsgMassSelectionTool.h>

#include <xAODBase/IParticle.h>

namespace CP
{

  StatusCode AsgMassSelectionTool ::
  initialize ()
  {
    if (m_minM < 0 || !std::isfinite (m_minM))
    {
      ATH_MSG_ERROR ("invalid value of minM: " << m_minM);
      return StatusCode::FAILURE;
    }
    if (m_maxM < 0 || !std::isfinite (m_maxM))
    {
      ATH_MSG_ERROR ("invalid value of maxM: " << m_maxM);
      return StatusCode::FAILURE;
    }

    if (m_minM > 0) {
       ATH_MSG_DEBUG( "Performing m >= " << m_minM << " MeV selection" );
       m_minMassCutIndex = m_accept.addCut ("minM", "minimum mass cut");
    }
    if (m_maxM > 0) {
       ATH_MSG_DEBUG( "Performing m < " << m_maxM << " MeV selection" );
       m_maxMassCutIndex = m_accept.addCut ("maxM", "maximum mass cut");
    }

    return StatusCode::SUCCESS;
  }


  const asg::AcceptInfo& AsgMassSelectionTool ::
  getAcceptInfo () const
  {
    return m_accept;
  }

  
  asg::AcceptData AsgMassSelectionTool ::
  accept (const xAOD::IParticle *particle) const
  {
    asg::AcceptData accept (&m_accept);

    // Perform the mass cuts.
    if (m_minMassCutIndex >= 0 || m_maxMassCutIndex >= 0)
    {
      float m = particle->m();

      if (m_minMassCutIndex >= 0) {
        if (!std::isfinite(m) || m < 0.)
        {
          ANA_MSG_WARNING ("invalid mass value, setting object to fail mass-cut: " << m);
          accept.setCutResult (m_minMassCutIndex, false);
        } else
        {
          accept.setCutResult (m_minMassCutIndex, m >= m_minM);
        }
      }
      if (m_maxMassCutIndex >= 0) {
        accept.setCutResult (m_maxMassCutIndex, m < m_maxM);
      }
    }

    return accept;
  }
}

