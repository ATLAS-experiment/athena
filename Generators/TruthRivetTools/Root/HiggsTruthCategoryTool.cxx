/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// TruthRivetTools includes
#include "TruthRivetTools/HiggsTruthCategoryTool.h"

#include <mutex>

HiggsTruthCategoryTool::HiggsTruthCategoryTool( const std::string& name)
: asg::AsgTool( name ),
  m_higgsTemplateCrossSections( nullptr )
{
  // cannot be set to true until the issue with the beam protons in the truth event record is fixed..
  // see JIRA ticket: https://its.cern.ch/jira/browse/ATLASRECTS-3072?filter=-2
  m_outHistos = false;
}


StatusCode HiggsTruthCategoryTool::initialize() {
  ATH_MSG_INFO ("Initializing " << name() << "...");
  // Rivet analysis :: Higgs truth event classifier class
  m_higgsTemplateCrossSections = new Rivet::HiggsTemplateCrossSections();
  // create an instance of the Rivet analysis handler
  m_rivetAnaHandler = std::make_unique<Rivet::AnalysisHandler>();
  // Add the Higgs truth classifier class to the handler
  m_rivetAnaHandler->addAnalysis(m_higgsTemplateCrossSections);
  return StatusCode::SUCCESS;
}

StatusCode HiggsTruthCategoryTool :: finalize () {
  ATH_MSG_INFO (" ====================================================== ");
  ATH_MSG_INFO (" ---- Finalizing" << name() << "...");
  ATH_MSG_INFO (" ====================================================== ");
  if ( !m_outHistos ){
    //this seems to thrown an exception
    //higgsTemplateCrossSections->printClassificationSummary( );
  }
  else{
    // TODO:: update the tool properly deal with output files/paths
    m_rivetAnaHandler->finalize();
    m_rivetAnaHandler->writeData("HiggsTruthCategoryTool.yoda");
  }
  ATH_MSG_INFO (" ====================================================== ");
  return StatusCode::SUCCESS;
}

HTXS::HiggsClassification* HiggsTruthCategoryTool :: getHiggsTruthCategoryObject (const HepMC::GenEvent& HepMCEvent, const HTXS::HiggsProdMode prodMode) const {
  if ( !m_isInitialized.test_and_set() ) {
    [&]() {
      m_higgsTemplateCrossSections->setHiggsProdMode(prodMode);
      m_rivetAnaHandler->init(HepMCEvent);
    }();
  }
  // fill histos if flag is specified
  // if ( m_outHistos ) rivetAnaHandler->analyze(HepMCEvent);

  // get the category output object containing the template cross section category,
  // and Higgs, V-boson, jets 4-vectors
  const Rivet::HiggsClassification htxs_cat_rivet = m_higgsTemplateCrossSections->classifyEvent(const_cast<HepMC::GenEvent&>(HepMCEvent),prodMode);
  return HTXS::Rivet2Root(htxs_cat_rivet);
}
