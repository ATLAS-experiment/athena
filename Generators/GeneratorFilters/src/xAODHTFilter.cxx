/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Header for this module
#include "GeneratorFilters/xAODHTFilter.h"
#include "GeneratorFilters/Common.h"

// Framework Related Headers
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SystemOfUnits.h"

// EDM includes
#include "xAODEventInfo/EventInfo.h"

// Used for retrieving the collection
#include "xAODJet/JetContainer.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthVertex.h"
#include "StoreGate/StoreGateSvc.h"
#include "StoreGate/WriteDecorHandle.h"

// Other classes used by this class
#include "TruthUtils/HepMCHelpers.h"
#include "AtlasHepMC/GenEvent.h"
// #include "GeneratorObjects/McEventCollection.h"
#include "TruthUtils/HepMCHelpers.h"

#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthParticleAuxContainer.h"
#include "GeneratorObjects/xAODTruthParticleLink.h"
// Tool handle interface
#include "MCTruthClassifier/IMCTruthClassifier.h"


StatusCode xAODHTFilter::filterInitialize()
{
  CHECK(m_TruthJetContainerName.initialize());
  CHECK(m_truthPartContKey.initialize());
  m_MinJetPt.value() /= Gaudi::Units::GeV;
  m_MinLepPt.value() /= Gaudi::Units::GeV;
  m_MinHT.value() /= Gaudi::Units::GeV;
  m_MaxHT.value() /= Gaudi::Units::GeV;
  if (m_MaxHT < 0)
    m_MaxHT = 9e9;

  ATH_MSG_INFO("Configured with " << m_MinJetPt << "<p_T GeV and abs(eta)<" << m_MaxJetEta << " for jets in " << m_TruthJetContainerName);
  ATH_MSG_INFO("Will require H_T in range " << m_MinHT << " < H_T < " << m_MaxHT);
  if (m_UseNu)
    ATH_MSG_INFO(" including neutrinos");
  if (m_UseLep)
    ATH_MSG_INFO(" including W/Z/tau leptons in range " << m_MinLepPt << "<p_T GeV and abs(eta)<" << m_MaxLepEta);

  CHECK(m_mcFilterHTKey.initialize());
  ATH_CHECK(m_classif.retrieve());

  return StatusCode::SUCCESS;
}

StatusCode xAODHTFilter::filterFinalize()
{
  ATH_MSG_INFO("Total efficiency: " << 100. * double(m_passed) / double(m_total) << "% ("
                                    << 100. * double(m_ptfailed) / double(m_total) << "% failed p_T cuts)");
  return StatusCode::SUCCESS;
}

StatusCode xAODHTFilter::filterEvent()
{
  m_total++; // Book keeping

  // Get jet container out
  SG::ReadHandle<xAOD::JetContainer>  truthjetTES{m_TruthJetContainerName};
  if (!truthjetTES.isValid()) {
    ATH_MSG_ERROR("No xAOD::JetContainer found in StoreGate with key " << m_TruthJetContainerName.key());
#ifdef HEPMC3
    setFilterPassed(m_MinHT < 1. || keepAll());
#else
    setFilterPassed(m_MinHT < 1.);
#endif
    return StatusCode::SUCCESS;
  }

  // Get HT
  double HT = -1;
  for (xAOD::JetContainer::const_iterator it_truth = (*truthjetTES).begin(); it_truth != (*truthjetTES).end(); ++it_truth)
  {
    if (!(*it_truth))
      continue;
    if ((*it_truth)->pt() > m_MinJetPt * Gaudi::Units::GeV && std::abs((*it_truth)->eta()) < m_MaxJetEta)
    {
      ATH_MSG_VERBOSE("Adding truth jet with pt " << (*it_truth)->pt()
                                                  << ", eta " << (*it_truth)->eta()
                                                  << ", phi " << (*it_truth)->phi()
                                                  << ", nconst = " << (*it_truth)->numConstituents());
      HT += (*it_truth)->pt();
    }
  }

  // If we are asked to include neutrinos or leptons...
  if (m_UseLep || m_UseNu)
  {

    // Retrieve TruthGen container from xAOD Gen slimmer, contains all particles witout barcode_zero and
    // duplicated barcode ones
    SG::ReadHandle<xAOD::TruthParticleContainer> xTruthParticleContainer{m_truthPartContKey};
    CHECK(xTruthParticleContainer.isValid());

    std::vector<const xAOD::TruthParticle *> WZleptons;
    WZleptons.reserve(10);

    // Loop over full TruthParticle container
    for (const xAOD::TruthParticle* theParticle : *xTruthParticleContainer) {
      if (!theParticle) continue;
      const int pdgid = theParticle->pdgId();
      if (m_UseNu && MC::isNeutrino(pdgid) && (theParticle->isGenStable())) {
        if (Common::prompt(theParticle,m_classif)) {
          HT += theParticle->pt();
        }
      }

      // pick muons and electrons specifically -- isLepton selects both charged leptons and neutrinos
      if ( m_UseLep && (MC::isElectron(pdgid) || MC::isMuon(pdgid)) && theParticle->isGenStable() && (theParticle)->pt() > m_MinLepPt * Gaudi::Units::GeV && std::abs(theParticle->eta()) < m_MaxLepEta) {
        if (Common::prompt(theParticle,m_classif)) {
          HT += theParticle->pt();
        }
      }
    } // End loop over particles
  }

  HT /= Gaudi::Units::GeV; // Make sure we're in GeV
  ATH_MSG_DEBUG("HT: " << HT);

#ifdef HEPMC3
    // fill the HT value
    // Event passed.  Will add HT to xAOD::EventInfo
    // Get MC event collection for setting weight
  const McEventCollection* mecc = 0;
    if ( evtStore()->retrieve( mecc ).isFailure() || !mecc ){ // FIXME keyless retrieve
      setFilterPassed(false);
      ATH_MSG_ERROR("Could not retrieve MC Event Collection - might not work");
      return StatusCode::SUCCESS;
    } 
  
    McEventCollection* mec = const_cast<McEventCollection*> (&(*mecc));
    for (unsigned int i = 0; i < mec->size(); ++i) {
      if (!(*mec)[i]) continue;
   
      (*mec)[i]->add_attribute("filterHT", std::make_shared<HepMC3::DoubleAttribute>(HT));
    }

  if ((HT < m_MinHT || HT >= m_MaxHT) && (!keepAll()))
#else
  if ((HT < m_MinHT || HT >= m_MaxHT) )
#endif
  {
    ATH_MSG_DEBUG("Failed filter on HT: " << HT << " is not between " << m_MinHT << " and " << m_MaxHT);
    setFilterPassed(false);
  }
  else
  {
   // Made it to the end - success! 
    m_passed++;
    setFilterPassed(true);
   }
  return StatusCode::SUCCESS;
}

