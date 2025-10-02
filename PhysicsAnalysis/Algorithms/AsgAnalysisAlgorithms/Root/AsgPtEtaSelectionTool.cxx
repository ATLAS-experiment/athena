/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <AsgAnalysisAlgorithms/AsgPtEtaSelectionTool.h>

#include <xAODEgamma/Egamma.h>
#include <xAODJet/Jet.h>
#include <xAODBase/IParticle.h>
#include <cmath>

//
// method implementations
//

namespace CP
{

  StatusCode AsgPtEtaSelectionTool ::
  initialize ()
  {
    if (bool{m_useDressedProperties} + bool{m_useClusterEta} + bool{m_useConstituentMomentum} > 1)
    {
      ATH_MSG_ERROR ("only one of 'useDressedProperties', 'useClusterEta' and 'useConstituentMomentum' can be used at the same time");
      return StatusCode::FAILURE;
    }
    if (m_minPt < 0 || !std::isfinite (m_minPt))
    {
      ATH_MSG_ERROR ("invalid value of minPt: " << m_minPt);
      return StatusCode::FAILURE;
    }
    if (m_maxPt < 0 || !std::isfinite (m_maxPt))
    {
      ATH_MSG_ERROR ("invalid value of maxPt: " << m_maxPt);
      return StatusCode::FAILURE;
    }
    if (m_minEta < 0 || !std::isfinite (m_minEta))
    {
      ATH_MSG_ERROR ("invalid value of minEta: " << m_minEta);
      return StatusCode::FAILURE;
    }
    if (m_maxEta < 0 || !std::isfinite (m_maxEta))
    {
      ATH_MSG_ERROR ("invalid value of maxEta: " << m_maxEta);
      return StatusCode::FAILURE;
    }
    if (m_etaGapLow < 0 || !std::isfinite (m_etaGapLow))
    {
      ATH_MSG_ERROR ("invalid value of etaGapLow: " << m_etaGapLow);
      return StatusCode::FAILURE;
    }
    if (m_etaGapHigh < 0 || !std::isfinite (m_etaGapHigh))
    {
      ATH_MSG_ERROR ("invalid value of etaGapHigh: " << m_etaGapHigh);
      return StatusCode::FAILURE;
    }
    if (m_etaGapHigh > 0 && m_etaGapLow >= m_etaGapHigh)
    {
      ATH_MSG_ERROR ("invalid eta gap: " << m_etaGapLow << " to " << m_etaGapHigh);
      return StatusCode::FAILURE;
    }
    if (m_etaGapLow > 0 && m_minEta > 0 && m_etaGapLow <= m_minEta)
    {
      ATH_MSG_ERROR ("etaGapLow=" << m_etaGapLow << " <= minEta=" << m_minEta);
      return StatusCode::FAILURE;
    }
    if (m_etaGapHigh > 0 && m_maxEta > 0 && m_etaGapHigh >= m_maxEta)
    {
      ATH_MSG_ERROR ("etaGapHigh=" << m_etaGapHigh << " >= maxEta=" << m_maxEta);
      return StatusCode::FAILURE;
    }
    if (m_minRapidity < 0 || !std::isfinite (m_minRapidity))
    {
      ATH_MSG_ERROR ("invalid value of minRapidity: " << m_minRapidity);
      return StatusCode::FAILURE;
    }
    if (m_maxRapidity < 0 || !std::isfinite (m_maxRapidity))
    {
      ATH_MSG_ERROR ("invalid value of maxRapidity: " << m_maxRapidity);
      return StatusCode::FAILURE;
    }
    if ((m_minRapidity > 0 && m_maxRapidity > 0) &&
        (m_minRapidity >= m_maxRapidity))
    {
      ATH_MSG_ERROR ("invalid rapidity range: " << m_minRapidity << " to " << m_maxRapidity);
      return StatusCode::FAILURE;
    }
    if ((m_minEta > 0 || m_maxEta > 0) && (m_minRapidity > 0 || m_maxRapidity > 0))
    {
      ATH_MSG_ERROR ("cannot use both eta and rapidity cuts at the same time");
      return StatusCode::FAILURE;
    }

    if (m_useDressedProperties) {
       ATH_MSG_DEBUG( "Performing pt and eta cuts on the dressed properties" );
       m_dressedPropertiesIndex = m_accept.addCut ("dressedProperties", "has dressed properties");
       m_dressedPtAccessor = std::make_unique<SG::ConstAccessor<float>> ("pt_dressed");
       m_dressedEtaAccessor = std::make_unique<SG::ConstAccessor<float>> ("eta_dressed");
    }
    if (m_minPt > 0) {
       ATH_MSG_DEBUG( "Performing pt >= " << m_minPt << " MeV selection" );
       m_minPtCutIndex = m_accept.addCut ("minPt", "minimum pt cut");
    }
    if (m_maxPt > 0) {
       ATH_MSG_DEBUG( "Performing pt < " << m_maxPt << " MeV selection" );
       m_maxPtCutIndex = m_accept.addCut ("maxPt", "maximum pt cut");
    }
    if (m_useClusterEta) {
       ATH_MSG_DEBUG( "Performing eta cut on the e/gamma cluster" );
       m_egammaCastCutIndex = m_accept.addCut ("castEgamma", "cast to egamma");
       m_egammaClusterCutIndex = m_accept.addCut ("caloCluster", "egamma object has cluster");
    }
    if (m_useConstituentMomentum) {
       ATH_MSG_DEBUG( "Performing eta/rapidity cut on the jet constituent momentum" );
       m_jetCastCutIndex = m_accept.addCut ("castJet", "cast to jet");
    }
    if (m_minEta > 0) {
       ATH_MSG_DEBUG( "Performing |eta| >= " << m_minEta << " selection");
       m_minEtaCutIndex = m_accept.addCut ("minEta", "minimum eta cut");
    }
    if (m_maxEta > 0) {
       ATH_MSG_DEBUG( "Performing |eta| < " << m_maxEta << " selection" );
       m_maxEtaCutIndex = m_accept.addCut ("maxEta", "maximum eta cut");
    }
    if (m_etaGapHigh > 0) {
       ATH_MSG_DEBUG( "Performing !( " << m_etaGapLow << " < |eta| < "
                      << m_etaGapHigh << " ) selection" );
       m_etaGapCutIndex = m_accept.addCut ("etaGap", "eta gap cut");
    }
    if (m_minRapidity > 0) {
       ATH_MSG_DEBUG( "Performing |rapidity| >= " << m_minRapidity << " selection");
       m_minRapidityCutIndex = m_accept.addCut ("minRapidity", "minimum eta cut");
    }
    if (m_maxRapidity > 0) {
       ATH_MSG_DEBUG( "Performing |rapidity| < " << m_maxRapidity << " selection" );
       m_maxRapidityCutIndex = m_accept.addCut ("maxRapidity", "maximum eta cut");
    }
    m_shouldPrintCastWarning = m_printCastWarning;
    m_shouldPrintClusterWarning = m_printClusterWarning;

    return StatusCode::SUCCESS;
  }



  const asg::AcceptInfo& AsgPtEtaSelectionTool ::
  getAcceptInfo () const
  {
    return m_accept;
  }



  asg::AcceptData AsgPtEtaSelectionTool ::
  accept (const xAOD::IParticle *particle) const
  {
    asg::AcceptData accept (&m_accept);

    // Check if dressed properties exist if needed
    if (m_useDressedProperties) {
       if (!m_dressedPtAccessor->isAvailable(*particle)) {
         ANA_MSG_WARNING ("dressed decorations not available");
         return accept;
       }
       accept.setCutResult (m_dressedPropertiesIndex, true);
    }

    // Perform the tranverse momentum cuts.
    if (m_minPtCutIndex >= 0 || m_maxPtCutIndex >= 0)
    {
      float pt = particle->pt();
      if (m_useDressedProperties) {
        pt = (*m_dressedPtAccessor) (*particle);
      }

      if (m_minPtCutIndex >= 0) {
        if (!std::isfinite(pt) || pt < 0.)
        {
          ANA_MSG_WARNING ("invalid pt value, setting object to fail pt-cut: " << pt);
          accept.setCutResult (m_minPtCutIndex, false);
        } else
        {
          accept.setCutResult (m_minPtCutIndex, pt >= m_minPt);
        }
      }
      if (m_maxPtCutIndex >= 0) {
        accept.setCutResult (m_maxPtCutIndex, pt < m_maxPt);
      }
    }

    // Perform the eta cut(s).
    if (m_minEtaCutIndex >= 0 || m_maxEtaCutIndex >= 0 || m_etaGapCutIndex >= 0)
    {
      float absEta = 0;

      if (m_useClusterEta == true)
      {
        const xAOD::Egamma *egamma
          = dynamic_cast<const xAOD::Egamma*>(particle);
        if (egamma == nullptr)
        {
          if (m_shouldPrintCastWarning)
            ANA_MSG_ERROR ("failed to cast input particle to electron");
          m_shouldPrintCastWarning = false;
          return accept;
        }
        accept.setCutResult (m_egammaCastCutIndex, true);
        const xAOD::CaloCluster *const caloCluster {egamma->caloCluster()};
        if (!caloCluster)
        {
          if (m_shouldPrintClusterWarning)
            ANA_MSG_ERROR ("no calo-cluster associated with e-gamma object");
          m_shouldPrintClusterWarning = false;
          return accept;
        }
        accept.setCutResult (m_egammaClusterCutIndex, true);
        absEta = std::abs (caloCluster->etaBE(2));
      } else if (m_useDressedProperties)
      {
        absEta = std::abs ((*m_dressedEtaAccessor) (*particle));
      } else if (m_useConstituentMomentum == true)
      {
        const xAOD::Jet *jet
          = dynamic_cast<const xAOD::Jet*>(particle);
        if (jet == nullptr)
        {
          if (m_shouldPrintCastWarning)
            ANA_MSG_ERROR ("failed to cast input particle to jet");
          m_shouldPrintCastWarning = false;
          return accept;
        }
        accept.setCutResult (m_jetCastCutIndex, true);
        absEta = std::abs (jet->getAttribute<xAOD::JetFourMom_t>("JetConstitScaleMomentum").eta());
      } else
      {
        absEta = std::abs (particle->eta());
      }

      if (m_minEtaCutIndex >= 0) {
        accept.setCutResult (m_minEtaCutIndex, absEta > m_minEta);
      }
      if (m_maxEtaCutIndex >= 0) {
        accept.setCutResult (m_maxEtaCutIndex, absEta <= m_maxEta);
      }
      if (m_etaGapCutIndex >= 0) {
        accept.setCutResult (m_etaGapCutIndex, (absEta < m_etaGapLow ||
                                                  absEta > m_etaGapHigh));
      }
    }

    // Perform the rapdity cut(s).
    if (m_minRapidityCutIndex >= 0 || m_maxRapidityCutIndex >= 0)
    {
      float absRapidity = 0;

      if (m_useConstituentMomentum == true)
      {
        const xAOD::Jet *jet
          = dynamic_cast<const xAOD::Jet*>(particle);
        if (jet == nullptr)
        {
          if (m_shouldPrintCastWarning)
            ANA_MSG_ERROR ("failed to cast input particle to jet");
          m_shouldPrintCastWarning = false;
          return accept;
        }
        accept.setCutResult (m_jetCastCutIndex, true);
        absRapidity = std::abs (jet->getAttribute<xAOD::JetFourMom_t>("JetConstitScaleMomentum").Rapidity());
      } else
      {
        absRapidity = std::abs (particle->rapidity());
      }

      if (m_minRapidityCutIndex >= 0) {
        accept.setCutResult (m_minRapidityCutIndex, absRapidity > m_minRapidity);
      }
      if (m_maxRapidityCutIndex >= 0) {
        accept.setCutResult (m_maxRapidityCutIndex, absRapidity <= m_maxRapidity);
      }
    }

    return accept;
  }
}
