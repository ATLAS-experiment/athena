/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Header for this module
#include "GeneratorFilters/xAODHTFilter.h"

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

//--------------------------------------------------------------------------

xAODHTFilter::xAODHTFilter(const std::string &name, ISvcLocator *pSvcLocator)
    : GenFilter(name, pSvcLocator), m_total(0), m_passed(0), m_ptfailed(0)
      , m_classif("MCTruthClassifier/DFCommonTruthClassifier")
{
  declareProperty("MinJetPt", m_MinJetPt = 0 * Gaudi::Units::GeV);
  declareProperty("MaxJetEta", m_MaxJetEta = 10.0);
  declareProperty("TruthJetContainer", m_TruthJetContainerName = "AntiKt4TruthWZJets");
  declareProperty("MinHT", m_MinHT = 20. * Gaudi::Units::GeV);
  declareProperty("MaxHT", m_MaxHT = 14000. * Gaudi::Units::GeV);
  declareProperty("UseNeutrinosFromWZTau", m_UseNu = false, "Include neutrinos from W/Z/tau decays in the calculation of HT");
  declareProperty("UseLeptonsFromWZTau", m_UseLep = false, "Include e/mu from W/Z/tau decays in the HT");
  declareProperty("MinLeptonPt", m_MinLepPt = 0 * Gaudi::Units::GeV);
  declareProperty("MaxLeptonEta", m_MaxLepEta = 10.0);
  declareProperty("EventInfoName",m_eventInfoName="EventInfo");   
}

//--------------------------------------------------------------------------

xAODHTFilter::~xAODHTFilter()
{
}

//---------------------------------------------------------------------------

StatusCode xAODHTFilter::filterInitialize()
{
  m_MinJetPt /= Gaudi::Units::GeV;
  m_MinLepPt /= Gaudi::Units::GeV;
  m_MinHT /= Gaudi::Units::GeV;
  m_MaxHT /= Gaudi::Units::GeV;
  if (m_MaxHT < 0)
    m_MaxHT = 9e9;

  ATH_MSG_INFO("Configured with " << m_MinJetPt << "<p_T GeV and abs(eta)<" << m_MaxJetEta << " for jets in " << m_TruthJetContainerName);
  ATH_MSG_INFO("Will require H_T in range " << m_MinHT << " < H_T < " << m_MaxHT);
  if (m_UseNu)
    ATH_MSG_INFO(" including neutrinos");
  if (m_UseLep)
    ATH_MSG_INFO(" including W/Z/tau leptons in range " << m_MinLepPt << "<p_T GeV and abs(eta)<" << m_MaxLepEta);

  ATH_CHECK(m_mcFilterHTKey.initialize());
  ATH_CHECK(m_classif.retrieve());

  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------

StatusCode xAODHTFilter::filterFinalize()
{
  ATH_MSG_INFO("Total efficiency: " << 100. * double(m_passed) / double(m_total) << "% ("
                                    << 100. * double(m_ptfailed) / double(m_total) << "% failed p_T cuts)");
  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------

StatusCode xAODHTFilter::filterEvent()
{
  m_total++; // Book keeping

  // Get jet container out
  const xAOD::JetContainer *truthjetTES = 0;
  if (!evtStore()->contains<xAOD::JetContainer>(m_TruthJetContainerName) ||
      evtStore()->retrieve(truthjetTES, m_TruthJetContainerName).isFailure() || !truthjetTES)
  {
    ATH_MSG_INFO("No xAOD::JetContainer found in StoreGate with key " << m_TruthJetContainerName);
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
  const xAOD::TruthParticleContainer* xTruthParticleContainer;
  if (evtStore()->retrieve(xTruthParticleContainer, "TruthGen").isFailure()) {
      ATH_MSG_ERROR("No TruthParticle collection with name " << "TruthGen" << " found in StoreGate!");
      return StatusCode::FAILURE;
  }

    std::vector<const xAOD::TruthParticle *> WZleptons;
    WZleptons.reserve(10);
    
    // Loop over full TruthParticle container
  unsigned int nPart = xTruthParticleContainer->size();
  for (unsigned int iPart = 0; iPart < nPart; ++iPart) {
        const xAOD::TruthParticle* theParticle =  (*xTruthParticleContainer)[iPart];
        if (!theParticle)
          continue;
        int pdgid = theParticle->pdgId();

        if (m_UseNu && MC::isNeutrino(pdgid) && (theParticle->isGenStable()))
        {
          if (isPrompt(theParticle))
          {
            HT += theParticle->pt();
          }
        }

        // pick muons and electrons specifically -- isLepton selects both charged leptons and neutrinos
        if ( m_UseLep && (std::abs(pdgid) == 11 || std::abs(pdgid) == 13) && theParticle->isGenStable() && (theParticle)->pt() > m_MinLepPt * Gaudi::Units::GeV && std::abs(theParticle->eta()) < m_MaxLepEta)
        {
          if (isPrompt(theParticle))
          {
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
    if ( evtStore()->retrieve( mecc ).isFailure() || !mecc ){
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

bool xAODHTFilter::isPrompt( const xAOD::TruthParticle *part ) const
  {
    MCTruthPartClassifier::ParticleOrigin orig = m_classif->particleTruthClassifier( part ).second;
    ATH_MSG_DEBUG("Particle has origin " << orig);

    switch(orig) {
    case MCTruthPartClassifier::NonDefined:
    case MCTruthPartClassifier::PhotonConv:
    case MCTruthPartClassifier::DalitzDec:
    case MCTruthPartClassifier::ElMagProc:
    case MCTruthPartClassifier::Mu:
    case MCTruthPartClassifier::LightMeson:
    case MCTruthPartClassifier::StrangeMeson:
    case MCTruthPartClassifier::CharmedMeson:
    case MCTruthPartClassifier::BottomMeson:
    case MCTruthPartClassifier::CCbarMeson:
    case MCTruthPartClassifier::JPsi:
    case MCTruthPartClassifier::BBbarMeson:
    case MCTruthPartClassifier::LightBaryon:
    case MCTruthPartClassifier::StrangeBaryon:
    case MCTruthPartClassifier::CharmedBaryon:
    case MCTruthPartClassifier::BottomBaryon:
    case MCTruthPartClassifier::PionDecay:
    case MCTruthPartClassifier::KaonDecay:
      return false;
    default:
      break;
    }

    return true;
  }

