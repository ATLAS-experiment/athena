/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "GeneratorFilters/QCDTruthJetFilter.h"
#include "GaudiKernel/PhysicalConstants.h"
#include "TRandom3.h"
#include "AthenaKernel/RNGWrapper.h"
#include "CLHEP/Random/RandomEngine.h"
#include "TF1.h" // For holding the weighting function

QCDTruthJetFilter::QCDTruthJetFilter(const std::string& name, ISvcLocator* pSvcLocator)
  : GenFilter(name,pSvcLocator)
{
}


StatusCode QCDTruthJetFilter::filterInitialize() {

  CHECK(m_TruthJetContainerName.initialize());
  CHECK(m_rndmSvc.retrieve());
  m_minPtCut = m_MinPt.value() / Gaudi::Units::GeV;
  m_maxPtCut = m_MaxPt.value() / Gaudi::Units::GeV;
  // Set actual Eta cut values based on configurable properties
  m_maxEtaCut = m_MaxEta.value();
  if (m_MinEta.value() == s_startMinEta && m_maxEtaCut>0) m_minEtaCut = -1.*m_maxEtaCut;
  if (m_SymEta) {
    ATH_MSG_INFO("Configured with " << m_minPtCut << "<p_T<" << m_maxPtCut << " GeV and " << m_minEtaCut << "<abs(eta)<" << m_maxEtaCut << " for jets in " << m_TruthJetContainerName);
  } else {
    ATH_MSG_INFO("Configured with " << m_minPtCut << "<p_T<" << m_maxPtCut << " GeV and " << m_minEtaCut << "<eta<" << m_maxEtaCut << " for jets in " << m_TruthJetContainerName);
  }

  // Special cases: min pT is above 2 TeV or max pT is below 20 GeV, use no weighting - hard-coded range of the fit in this filter
  if (m_minPtCut < 1999. && m_maxPtCut > 21. && m_doShape) {
    // Use the fit in all its glory
    m_high = fitFn(m_maxPtCut);
    m_norm = 1./(m_high==0?1.:m_high); //(m_maxPtCut-m_minPtCut) / (m_norm!=0?m_norm:1.);
    ATH_MSG_INFO("Initialized and set high to " << m_high << " and norm to " << m_norm);
  } else if (m_doShape) {
    ATH_MSG_INFO("Requested shaping with bounds of " << m_minPtCut << " , " << m_maxPtCut << " which cannot be done.  Turning off shaping (sorry).");
    m_doShape=false; // FIXME Do not alter the values of configurable properties in initialize
  }

if (m_MinPhi > -998.0 || m_MaxPhi < 998.0) {
    ATH_MSG_INFO("Configured phi as well with min = " << m_MinPhi << " and max = " << m_MaxPhi);
  }

  return StatusCode::SUCCESS;
}


StatusCode QCDTruthJetFilter::filterFinalize() {
  ATH_MSG_INFO("Total efficiency: " << 100.*double(m_passed)/double(m_total) << "% (" << 100.*double(m_ptfailed)/double(m_total) << "% failed pT cuts)");
  return StatusCode::SUCCESS;
}


StatusCode QCDTruthJetFilter::filterEvent() {
  m_total++; // Bookkeeping

  // Grab random number engine
  const EventContext& ctx = Gaudi::Hive::currentContext();
  CLHEP::HepRandomEngine* rndm = this->getRandomEngine(name(), ctx);
  if (!rndm) {
    ATH_MSG_WARNING("Failed to retrieve random number engine QCDTruthJetFilter");
    setFilterPassed(false);
    return StatusCode::SUCCESS;
  }

  // Get jet container out
    // Get jet container out
  SG::ReadHandle<xAOD::JetContainer>  truthjetTES{m_TruthJetContainerName};
  if (!truthjetTES.isValid()) {
    ATH_MSG_ERROR("No xAOD::JetContainer found in StoreGate with key " << m_TruthJetContainerName.key());
    setFilterPassed(m_minPtCut < 1);
    return StatusCode::SUCCESS;
  }

  // Get pT of leading jet
  double pt_lead = -1, phi_lead = 0;
  for (xAOD::JetContainer::const_iterator it_truth = (*truthjetTES).begin(); it_truth != (*truthjetTES).end() ; ++it_truth) {
    if (!(*it_truth)) continue;
    if ( ( m_SymEta && !(std::abs((*it_truth)->eta())>=m_minEtaCut && std::abs((*it_truth)->eta())<m_maxEtaCut) )
	 || ( !m_SymEta && !((*it_truth)->eta()>=m_minEtaCut && (*it_truth)->eta()<m_maxEtaCut) )) continue;
    if (pt_lead < (*it_truth)->pt()) {
      pt_lead = (*it_truth)->pt();
      phi_lead = (*it_truth)->phi();
    }
  }

  pt_lead /= Gaudi::Units::GeV; // Make sure we're in GeV

  // See if the leading jet is in the right range
  if ((pt_lead<=m_minPtCut || (pt_lead>m_maxPtCut && m_maxPtCut>0)) && !(pt_lead<=m_minPtCut && m_minPtCut<1)) {
    m_ptfailed++;
    setFilterPassed(false);
    ATH_MSG_DEBUG("Failed filter on jet pT: " << pt_lead << " is not between " << m_minPtCut << " and " << m_maxPtCut);
    return StatusCode::SUCCESS;
  }

// If appropriate, check the phi of the lead jet as well
  if (m_MinPhi > -999.0 || m_MaxPhi < 999.0) {
    setFilterPassed(false);

    if (phi_lead < m_MinPhi || phi_lead > m_MaxPhi) {
      ATH_MSG_DEBUG("Failed filter on jet phi: " << phi_lead << " not between " << m_MinPhi << " and " << m_MaxPhi);
      return StatusCode::SUCCESS;
    }
    else {
      setFilterPassed(true);
    }
  }


  // Unweight the pT spectrum
  double w = 1.;
  if (m_doShape) w = fitFn(pt_lead);
  double rnd = rndm->flat();
  if (m_high/w < rnd) {
    setFilterPassed(false);
    ATH_MSG_DEBUG("Event failed weighting cut. Weight is " << w << " for pt_lead of " << pt_lead << " high end is " << m_high << " rnd is " << rnd);
    return StatusCode::SUCCESS;
  }

  // Made it to the end - success!
  m_passed++;
  setFilterPassed(true);

  // Get MC event collection for setting weight
  const McEventCollection* mecc = 0;
  CHECK(evtStore()->retrieve(mecc, m_mcEventKey));
  ATH_MSG_DEBUG("Event passed.  Will mod event weights by " << w*m_norm << " for pt_lead of " << pt_lead << " norm " << m_norm << " w " << w << " high " << m_high << " rnd " << rnd);
  double orig = 1.;
  McEventCollection* mec = const_cast<McEventCollection*> (&(*mecc));
  for (unsigned int i=0;i<mec->size();++i) {
    if ( !(*mec)[i] ) continue;
    orig = (*mec)[i]->weights().size()>0?(*mec)[i]->weights()[0]:1.;
    if ((*mec)[i]->weights().size()>0) (*mec)[i]->weights()[0] = orig*w*m_norm;
    else (*mec)[i]->weights().push_back( w*m_norm*orig );
    
#ifdef HEPMC3
      (*mec)[i]->add_attribute("filterWeight", std::make_shared<HepMC3::DoubleAttribute>(w*m_norm));
#endif

  }
  return StatusCode::SUCCESS;
}


CLHEP::HepRandomEngine* QCDTruthJetFilter::getRandomEngine(const std::string& streamName,
                                                           const EventContext& ctx) const
{
  ATHRNG::RNGWrapper* rngWrapper = m_rndmSvc->getEngine(this, streamName);
  std::string rngName = name()+streamName;
  rngWrapper->setSeed( rngName, ctx );
  return rngWrapper->getEngine(ctx);
}
