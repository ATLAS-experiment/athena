/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):
#include "xAODTau/TauJetContainer.h"
#include "AthContainers/ConstAccessor.h"

// Local include(s):
#include "TauAnalysisToolsExampleAthena.h"

using namespace TauAnalysisTools;

TauAnalysisToolsExampleAthena::TauAnalysisToolsExampleAthena( const std::string& name, ISvcLocator* svcLoc )
  : AthAlgorithm( name, svcLoc )
{
}

StatusCode TauAnalysisToolsExampleAthena::initialize()
{
  // Greet the user:
  ATH_MSG_INFO( "Initialising" );
  ATH_MSG_DEBUG( "TauJets SGKey = " << m_sgKey_TauJets );
  if (m_useMuonRemovalTaus){ 
    ATH_MSG_DEBUG( "TauJets_MuonRM SGKey = " << m_sgKey_TauJets_MuonRM );
  }
  ATH_MSG_DEBUG( "TauSelectionTool  = " << m_selTool );
  ATH_MSG_DEBUG( "TauSmearingTool   = " << m_smearTool );
  ATH_MSG_DEBUG( "TauEfficiencyTool = " << m_effTool );

  // Retrieve the tools:
  ATH_CHECK( m_selTool.retrieve() );
  ATH_CHECK( m_smearTool.retrieve() );
  ATH_CHECK( m_effTool.retrieve() );

  // Return gracefully:
  return StatusCode::SUCCESS;
}

StatusCode TauAnalysisToolsExampleAthena::execute()
{
  // Retrieve the taus:
  std::vector<const xAOD::TauJet*> taus_to_calibrate;
  const xAOD::TauJetContainer* taus_std = 0;
  ATH_CHECK( evtStore()->retrieve( taus_std, m_sgKey_TauJets ) );
  ATH_MSG_INFO( "Number of taus: " << taus_std->size() );
  const xAOD::TauJetContainer* taus_muonRM = 0;
  if (m_useMuonRemovalTaus){
    ATH_CHECK( evtStore()->retrieve( taus_muonRM, m_sgKey_TauJets_MuonRM ) );
    ATH_MSG_INFO( "Number of muon removal taus: " << taus_muonRM->size() );
    taus_to_calibrate = TauAnalysisTools::combineTauJetsWithMuonRM(taus_std, taus_muonRM);
  }
  else {
    for (const xAOD::TauJet* tau : *taus_std){
      taus_to_calibrate.push_back(tau);
    }
  }
  // Loop over them:
  for(const xAOD::TauJet* tau_uncali : taus_to_calibrate)
  {
    ATH_MSG_DEBUG( "  current tau: eta = " << tau_uncali->eta()
                   << ", phi = " << tau_uncali->phi()
                   << ", pt = " << tau_uncali->pt() );

    // Select "good" taus:
    if( ! m_selTool->accept( *tau_uncali ) ) continue;

    // Print some info about the selected tau:
    ATH_MSG_INFO( "  Selected tau: eta = " << tau_uncali->eta()
                  << ", phi = " << tau_uncali->phi()
                  << ", pt = " << tau_uncali->pt() );

    // copy constant objects to non-constant
    xAOD::TauJet* tau = 0;
    tau = new xAOD::TauJet();
    tau->makePrivateStore( *tau_uncali );

    ATH_CHECK (m_effTool->applyEfficiencyScaleFactor(*tau));

    static const SG::ConstAccessor<double> accTauScaleFactorJetID ("TauScaleFactorJetIDHadTau");
    ATH_MSG_INFO( "  sf = " << accTauScaleFactorJetID (*tau) );

    ATH_CHECK (m_smearTool->applyCorrection(*tau));
    ATH_MSG_INFO( "Unsmeared tau pt " << tau->pt() << " Smeared tau pt: " << tau->p4().Pt());
  }

  // Return gracefully:
  return StatusCode::SUCCESS;
}
