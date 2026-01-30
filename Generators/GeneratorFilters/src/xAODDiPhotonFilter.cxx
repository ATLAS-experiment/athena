/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "GeneratorFilters/xAODDiPhotonFilter.h"
#include "TruthUtils/HepMCHelpers.h"
#include <vector>
#include "TMath.h"

// Pt High --> Low
/// @todo Move to a sorting utils module
class High2LowByGenParticleClassPt {
public:
  //bool operator() (const HepMC::ConstGenParticlePtr& t1, const HepMC::ConstGenParticlePtr& t2) const {
    bool operator() (const xAOD::TruthParticle *t1, const xAOD::TruthParticle *t2) const {
    return t1->pt() > t2->pt();
    //return t1->momentum().perp2() > t2->momentum().perp2();
  }
};

StatusCode xAODDiPhotonFilter::filterInitialize() {
  ATH_MSG_INFO("*** filter condition ***");
  CHECK(m_truthPartContKey.initialize());
  
  ATH_MSG_INFO("At least two photons must be in an event.");
  ATH_MSG_INFO("PtCut for the 1st photon  = " << m_Ptmin_1st);
  ATH_MSG_INFO("PtCut for the 2nd photon  = " << m_Ptmin_2nd );
  ATH_MSG_INFO("PtCut for other photons   = " << m_Ptmin_others);
  ATH_MSG_INFO("EtaCut for the 1st photon = " << m_EtaRange_1st);
  ATH_MSG_INFO("EtaCut for the 2nd photon = " << m_EtaRange_2nd);
  ATH_MSG_INFO("EtaCut for other photons  = " << m_EtaRange_others);
  ATH_MSG_INFO("DeltaRCut(min)            = " << m_diphoton_deltaRmin);
  ATH_MSG_INFO("DeltaRCut(max)            = " << m_diphoton_deltaRmax);
  ATH_MSG_INFO("MassCut(min)              = " << m_diphoton_massmin);
  ATH_MSG_INFO("MassCut(max)              = " << m_diphoton_massmax);
  ATH_MSG_INFO("Diphoton Pt cut(min)      = " << m_diphoton_PtMin);
  ATH_MSG_INFO("Diphoton Pt cut(max)      = " << m_diphoton_PtMax );
  ATH_MSG_INFO("   negative value on MassCut(min,max) -> no limit in the cut");
  ATH_MSG_INFO("Use only the 1st and the 2nd photons for mass and deltaR cuts, flag = " << m_use1st2ndPhotonsforMassAndDeltaRCuts);
  
  return StatusCode::SUCCESS;
}


StatusCode xAODDiPhotonFilter::filterEvent() {
  // get min pt
  double ptcut = m_Ptmin_1st > m_Ptmin_2nd ? m_Ptmin_2nd : m_Ptmin_1st;
  if (ptcut > m_Ptmin_others) ptcut = m_Ptmin_others;

  ATH_MSG_DEBUG("min pt(photon) = " << ptcut << " (CLHEP::MeV)");

 
 // Retrieve Photon container
 SG::ReadHandle<xAOD::TruthParticleContainer> xTruthParticleContainer{m_truthPartContKey};
 CHECK(xTruthParticleContainer.isValid());
 // find truth photons
 std::vector<const xAOD::TruthParticle *> MCTruthPhotonList;
   
 for (const xAOD::TruthParticle* part : *xTruthParticleContainer) {
      if ( MC::isPhoton(part) ) {
        if ( MC::isStable(part) && (part->pt() >= ptcut) ) {
          MCTruthPhotonList.push_back(part);
        }
      }
    }
 

  // sort truth photons with pT
  std::sort(MCTruthPhotonList.begin(), MCTruthPhotonList.end(), High2LowByGenParticleClassPt());

  // check conditions
  bool isOK = true;
  ATH_MSG_DEBUG("# of truth photons = " << MCTruthPhotonList.size());
  if (MCTruthPhotonList.size() < 2) {
    isOK = false;
  } else {
    
    std::vector<const xAOD::TruthParticle *> MCTruthPhotonList2;
    // check pT and eta to select truth photons
    for (size_t i = 0; i < MCTruthPhotonList.size(); ++i) {
      
      ATH_MSG_DEBUG(i << ": pT=" << MCTruthPhotonList[i]->pt() << ", eta=" << MCTruthPhotonList[i]->abseta());
      double ptmin  = m_Ptmin_others;
      double etamax = m_EtaRange_others;
      if (MCTruthPhotonList2.size() == 0) {
        ptmin  = m_Ptmin_1st;
        etamax = m_EtaRange_1st;
      } else if (MCTruthPhotonList2.size() == 1) {
        ptmin  = m_Ptmin_2nd;
        etamax = m_EtaRange_2nd;
      }
      
      if (MCTruthPhotonList[i]->pt() >= ptmin &&
        std::abs(MCTruthPhotonList[i]->abseta()) <= etamax) {
        MCTruthPhotonList2.push_back(MCTruthPhotonList[i]);
      }
    }
    ATH_MSG_DEBUG("# of truth photons after pT and eta cut = " << MCTruthPhotonList2.size());

    if (MCTruthPhotonList2.size() < 2) {
      isOK = false;
    } else {
      int nGood = 0;
      if (m_use1st2ndPhotonsforMassAndDeltaRCuts) {
        double sumPx = MCTruthPhotonList2[0]->px()+MCTruthPhotonList2[1]->px();
        double sumPy = MCTruthPhotonList2[0]->py()+MCTruthPhotonList2[1]->py();
        double sumPz = MCTruthPhotonList2[0]->pz()+MCTruthPhotonList2[1]->pz();
        double sumE  = MCTruthPhotonList2[0]->e() +MCTruthPhotonList2[1]->e();
        double m2 = sumE*sumE-(sumPx*sumPx+sumPy*sumPy+sumPz*sumPz);
        double mGamGam = m2 >= 0. ? std::sqrt(m2) : -std::sqrt(-m2);
        ATH_MSG_DEBUG("mass(gamgam) = " << mGamGam << " (CLHEP::MeV)");
        double deltaEta = MCTruthPhotonList2[0]->abseta() - MCTruthPhotonList2[1]->abseta();
        double deltaPhi = MCTruthPhotonList2[0]->phi() - MCTruthPhotonList2[1]->phi();
        double deltaR = std::sqrt(deltaEta*deltaEta+deltaPhi*deltaPhi);
        ATH_MSG_DEBUG("deltaR(gamgam) = " << deltaR);
        int testMassDeltaRCuts = 0;
        
        // compute diphoton pT
        double diphotonPt = std::sqrt(sumPx*sumPx + sumPy*sumPy);
        ATH_MSG_DEBUG("diphoton Pt = " << diphotonPt << " (MeV)");

        // apply diphoton pT cut
        if (m_diphoton_PtMin >= 0. && diphotonPt < m_diphoton_PtMin) {
            setFilterPassed(false);
            return StatusCode::SUCCESS;
        }
        if (m_diphoton_PtMax >= 0. && diphotonPt > m_diphoton_PtMax) {
            setFilterPassed(false);
            return StatusCode::SUCCESS;
        }

        //check mass
        if (m_diphoton_massmin >= 0. && m_diphoton_massmax >= 0.) {
          if (mGamGam >= m_diphoton_massmin && mGamGam <= m_diphoton_massmax) ++testMassDeltaRCuts;
        } else if (m_diphoton_massmin >= 0. && m_diphoton_massmax <  0.) {
          if (mGamGam >= m_diphoton_massmin) ++testMassDeltaRCuts;
        } else if (m_diphoton_massmin <  0. && m_diphoton_massmax >= 0.) {
          if (mGamGam <= m_diphoton_massmax) ++testMassDeltaRCuts;
        } else {
          ++testMassDeltaRCuts;
        }
        // check deltaR
        if (m_diphoton_deltaRmin >= 0. && m_diphoton_deltaRmax >= 0.) {
          if (deltaR >= m_diphoton_deltaRmin && deltaR <= m_diphoton_deltaRmax) ++testMassDeltaRCuts;
        } else if (m_diphoton_deltaRmin >= 0. && m_diphoton_deltaRmax < 0.) {
          if (deltaR >= m_diphoton_deltaRmin) ++testMassDeltaRCuts;
        } else if (m_diphoton_deltaRmin < 0. && m_diphoton_deltaRmax >= 0.) {
          if (deltaR <= m_diphoton_deltaRmax) ++testMassDeltaRCuts;
        } else {
          ++testMassDeltaRCuts;
        }
        // count pairs
        if (testMassDeltaRCuts == 2) ++nGood;
      } else {
        for (size_t i=0;i<MCTruthPhotonList2.size()-1;++i) {
          for (size_t j=i+1;j<MCTruthPhotonList2.size();++j) {
            double sumPx = MCTruthPhotonList2[i]->px()+MCTruthPhotonList2[j]->px();
            double sumPy = MCTruthPhotonList2[i]->py()+MCTruthPhotonList2[j]->py();
            double sumPz = MCTruthPhotonList2[i]->pz()+MCTruthPhotonList2[j]->pz();
            double sumE  = MCTruthPhotonList2[i]->e() +MCTruthPhotonList2[j]->e();
            double m2 = sumE*sumE-(sumPx*sumPx+sumPy*sumPy+sumPz*sumPz);
            double mGamGam = m2 >= 0. ? std::sqrt(m2) : -std::sqrt(-m2);
            ATH_MSG_DEBUG("mass(gamgam) = " << mGamGam << " (CLHEP::MeV)");
            double deltaEta = MCTruthPhotonList2[i]->abseta() - MCTruthPhotonList2[j]->abseta();
            double deltaPhi = MCTruthPhotonList2[i]->phi() - MCTruthPhotonList2[j]->phi();
            double deltaR = std::sqrt(deltaEta*deltaEta+deltaPhi*deltaPhi);
            ATH_MSG_DEBUG("deltaR(gamgam) = " << deltaR);
            int testMassDeltaRCuts = 0;
            
            // compute diphoton pT
            double diphotonPt = std::sqrt(sumPx*sumPx + sumPy*sumPy);
            ATH_MSG_DEBUG("diphoton Pt = " << diphotonPt << " (MeV)");

            // apply diphoton pT cut
            if (m_diphoton_PtMin >= 0. && diphotonPt < m_diphoton_PtMin) continue;
            if (m_diphoton_PtMax >= 0. && diphotonPt > m_diphoton_PtMax) continue;

            // check mass
            if (m_diphoton_massmin >= 0. && m_diphoton_massmax >= 0.) {
              if (mGamGam >= m_diphoton_massmin && mGamGam <= m_diphoton_massmax) ++testMassDeltaRCuts;
            } else if (m_diphoton_massmin >= 0. && m_diphoton_massmax <  0.) {
              if (mGamGam >= m_diphoton_massmin) ++testMassDeltaRCuts;
            } else if (m_diphoton_massmin <  0. && m_diphoton_massmax >= 0.) {
              if (mGamGam <= m_diphoton_massmax) ++testMassDeltaRCuts;
            } else {
              ++testMassDeltaRCuts;
            }
            // check deltaR
            if (m_diphoton_deltaRmin >= 0. && m_diphoton_deltaRmax >= 0.) {
              if (deltaR >= m_diphoton_deltaRmin && deltaR <= m_diphoton_deltaRmax) ++testMassDeltaRCuts;
            } else if (m_diphoton_deltaRmin >= 0. && m_diphoton_deltaRmax < 0.) {
              if (deltaR >= m_diphoton_deltaRmin) ++testMassDeltaRCuts;
            } else if (m_diphoton_deltaRmin < 0. && m_diphoton_deltaRmax >= 0.) {
              if (deltaR <= m_diphoton_deltaRmax) ++testMassDeltaRCuts;
            } else {
              ++testMassDeltaRCuts;
            }
            // count pairs
            if (testMassDeltaRCuts == 2) ++nGood;
          }
        }
      }

      ATH_MSG_DEBUG("# of good photon pair = " << nGood);
      if (nGood == 0) isOK = false;
    }
  }

  ATH_MSG_DEBUG("flag(final decision) = " << isOK);
  setFilterPassed(isOK);
  return StatusCode::SUCCESS;
}
