/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "GeneratorFilters/xAODJetFilter.h"

#include <algorithm>
#include <functional>
#include <vector>

#include "GaudiKernel/PhysicalConstants.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthParticleAuxContainer.h"
#include "xAODTruth/TruthParticleContainer.h"

xAODJetFilter::xAODJetFilter(const std::string& name, ISvcLocator* pSvcLocator)
    : GenFilter(name, pSvcLocator) {}

StatusCode xAODJetFilter::filterInitialize() {
  m_emaxeta = 6.0;
  m_edphi = 2 * M_PI / m_grphi;         // cell size
  m_edeta = 2. * m_emaxeta / m_greta;  // cell size
  // How many cells in the jet cluster
  if (!m_type) {  // it's the rectangular grid
    m_nphicell = m_gridp / 2;
    m_netacell = m_gride / 2;
    m_nphicell2 = m_gridp;
    m_netacell2 = m_gride;
  } else {
    m_nphicell = static_cast<int>(m_cone / m_edphi);  // number of cells inside cone
    m_netacell = static_cast<int>(m_cone / m_edeta);  // number of cells inside come
    m_nphicell2 =
        2 * m_nphicell +
        1;  // guaranteed to be odd so that the highest cell is in middle
    m_netacell2 = 2 * m_netacell + 1;
  }
  ATH_MSG_INFO("Parameters are \n ");
  if (m_type) {
    ATH_MSG_INFO("  Cone algorithm: "
                 << " Pt cut  = " << m_userThresh
                 << ", Number= " << m_userNumber << ", Cone size=" << m_cone
                 << ", Rapidity range " << m_userEta);
  } else {
    ATH_MSG_INFO("  GridAlgorithm: "
                 << " Pt cut  = " << m_userThresh << ", Number= "
                 << m_userNumber << ", eta size (units of 0.06) =" << m_gride
                 << ", phi size (units of 0.06) =" << m_gridp
                 << ", Rapidity range " << m_userEta);
  }

  return StatusCode::SUCCESS;
}

StatusCode xAODJetFilter::filterEvent() {
  // Init grid
  double etgrid[m_grphi][m_greta];    // clean it out before we start
  bool etgridused[m_grphi][m_greta];  // will use this to mark off cells after
                                      // they are added to jets
  for (int ie = 0; ie < m_greta; ++ie) {  // initialise everything to be safe
    for (int ip = 0; ip < m_grphi; ++ip) {
      etgrid[ip][ie] = 0.;
      etgridused[ip][ie] = false;
    }
  }

// Retrieve TruthGen container from xAOD Gen slimmer, contains all particles witout barcode_zero and 
// duplicated barcode ones
  const xAOD::TruthParticleContainer* xTruthParticleContainer;
  if (evtStore()->retrieve(xTruthParticleContainer, "TruthGen").isFailure()) {
      ATH_MSG_ERROR("No TruthParticle collection with name " << "TruthGen" << " found in StoreGate!");
      return StatusCode::FAILURE;
  }
  
  // Loop over all particles in the event and build up the grid

  unsigned int nPart = xTruthParticleContainer->size();
  for (unsigned int iPart = 0; iPart < nPart; ++iPart) {
      const xAOD::TruthParticle* part =  (*xTruthParticleContainer)[iPart];

      if (part->isGenStable()) {  // stables only
        if ((part->pdgId() != 13) && (part->pdgId() != -13) &&
            (part->pdgId() != 12) && (part->pdgId() != -12) &&
            (part->pdgId() != 14) && (part->pdgId() != -14) &&
            (part->pdgId() != 16) && (part->pdgId() != -16) &&
            (std::abs(part->eta()) <=
             m_emaxeta)) {  // no neutrinos or muons and particles must be in
                            // active range
          int ip, ie;
          ip = static_cast<int>((M_PI + part->phi()) /
                     m_edphi);  // phi is in range -CLHEP::pi to CLHEP::pi
          ie = static_cast<int>((part->eta() + m_emaxeta) / m_edeta);
          if (ie < 0 ||
              (ie >=
               m_greta)) {  // its outside the ends so we should not be here
            ATH_MSG_FATAL("Jet too close to end");
            return StatusCode::FAILURE;
          }
          while (ip < 0)
            ip += m_grphi;  // fix phi wrapping note that this is done after rr
                            // is calculated
          while (ip > m_grphi - 1)
            ip -= m_grphi;  // fix phi wrapping note that this is done after rr
                            // is calculated
          etgrid[ip][ie] = etgrid[ip][ie] + part->pt();  // fortran had pt here
        }
    }
  } // end Particles loop

  // Find the highest cell; we loop here until we cannot find more jets
  double ethigh = 2. * m_stop;  // et of highest cell
  while (
      ethigh >
      m_stop) {  // stop looping when there are no cells left above threshold;
    ethigh = 0.;
    int etahigh = 0;
    int phihigh = 0;
    for (int ie0 = m_netacell; ie0 < m_greta - m_netacell;
         ++ie0) {  // only look away from the edges
      for (int ip0 = 0; ip0 < m_grphi; ++ip0) {
        if (etgrid[ip0][ie0] > ethigh && !etgridused[ip0][ie0]) {
          ethigh = etgrid[ip0][ie0];
          etahigh = ie0;
          phihigh = ip0;
        }
      }
    }

    if (ethigh >
        m_stop) {  // this passes only if there is tower above threshold
      // new jet
      CLHEP::HepLorentzVector FoundJet;
      double jetpx = 0.;
      double jetpy = 0.;
      double jetpz = 0.;
      double jete = 0.;
      if (!m_type) {  // grid handle differantly if there are an even number of
                      // cells
        if (m_netacell2 % 2 == 0 && m_nphicell2 % 2 == 1) {  // eta even
          double sum = 0.;
          double sum1 = 0.;
          for (int ip0 = 0; ip0 < m_nphicell2; ip0++) {
            int ip1 = ip0 - m_nphicell + phihigh;
            sum = sum + etgrid[ip1][etahigh - 1];
            sum1 = sum1 + etgrid[ip1][etahigh + 1];
          }
          if (sum < sum1) {
            etahigh += 1;  // shift over by one
          }
        }
        if (m_netacell2 % 2 == 1 && m_nphicell2 % 2 == 0) {  // phi even
          double sum = 0.;
          double sum1 = 0.;
          for (int ie0 = 0; ie0 < m_netacell2; ie0++) {
            int ie1 = ie0 - m_netacell + etahigh;
            sum = sum + etgrid[(phihigh - 1) % m_grphi][ie1];
            sum1 = sum1 + etgrid[(phihigh + 1) % m_grphi][ie1];
          }
          if (sum < sum1) {
            phihigh = (phihigh + 1) % m_grphi;  // shift over by one
          }
        }
        if (m_netacell2 % 2 == 0 && m_nphicell2 % 2 == 0) {  // both even
          double sum = 0.;
          double sum1 = 0.;
          double sum2 = 0.;
          double sum3 = 0.;
          for (int ie0 = 0; ie0 < m_netacell2; ie0++) {
            for (int ip0 = 0; ip0 < m_nphicell2; ip0++) {
              int ip1 = ip0 - m_nphicell + phihigh;
              int ie1 = ie0 - m_netacell + etahigh;
              if (!etgridused[ip1][ie1])
                sum = sum + etgrid[ip1][ie1];
              if (!etgridused[ip1][ie1 + 1])
                sum1 = sum1 + etgrid[ip1][ie1 + 1];
              if (!etgridused[ip1 + 1][ie1])
                sum2 = sum2 + etgrid[(ip1 + 1) % m_grphi][ie1];
              if (!etgridused[ip1 + 1][ie1 + 1])
                sum3 = sum3 + etgrid[(ip1 + 1) % m_grphi][ie1 + 1];
            }
          }
          if (sum < sum1 && sum2 < sum1 && sum3 < sum1)
            etahigh = etahigh + 1;
          if (sum < sum2 && sum1 <= sum2 && sum3 < sum2)
            phihigh = (phihigh + 1) % m_grphi;
          if (sum < sum3 && sum2 <= sum3 && sum1 <= sum3) {
            etahigh = etahigh + 1;
            phihigh = (phihigh + 1) % m_grphi;
          }
        }
      }
      // Add up stuff around high cell
      for (int ie0 = 0; ie0 < m_netacell2; ++ie0) {
        int ie1 = ie0 - m_netacell + etahigh;
        if ((ie1 < 0) ||
            (ie1 >=
             m_greta)) {  // its outside the ends so we should not be here
          ATH_MSG_FATAL("  Jet too close to end");
          return StatusCode::FAILURE;
        }
        for (int ip0 = 0; ip0 < m_nphicell2; ++ip0) {
          int ip1 = ip0 - m_nphicell + phihigh;
          // are we using a cone, if so check that its inside
          double rr = (ie1 - etahigh) * (ie1 - etahigh) * m_edeta * m_edeta +
                      (ip1 - phihigh) * (ip1 - phihigh) * m_edphi * m_edphi;
          while (ip1 < 0)
            ip1 += m_grphi;  // fix phi wrapping note that this is done after rr
                             // is calculated
          while (ip1 > m_grphi - 1)
            ip1 -= m_grphi;  // fix phi wrapping note that this is done after rr
                             // is calculated
          if (rr < m_cone * m_cone || !m_type) {  // make sure that its inside
            // check that the cell can be used and add energy to jet and mark
            // the cell as used
            if (!etgridused[ip1][ie1]) {
              etgridused[ip1][ie1] = true;
              jetpx = jetpx + etgrid[ip1][ie1] *
                                  std::cos(-M_PI + (ip1 + 0.5) * m_edphi);
              jetpy = jetpy + etgrid[ip1][ie1] *
                                  std::sin(-M_PI + (ip1 + 0.5) * m_edphi);
              jetpz = jetpz + etgrid[ip1][ie1] *
                                  std::sinh((ie1 + 0.5) * m_edeta - m_emaxeta);
              jete = jete +
                     etgrid[ip1][ie1] * std::cosh((ie1 + 0.5) * m_edeta - m_emaxeta);
            }
          }
        }
      }
      FoundJet.setPx(jetpx);
      FoundJet.setPy(jetpy);
      FoundJet.setPz(jetpz);
      FoundJet.setE(jete);
      if (std::abs(FoundJet.pseudoRapidity()) < m_userEta) {
        m_Jets.push_back(FoundJet);  // OK we found one. add it to the list  if
                                     // its inside the eta region
      }
    }
  }
  sort(m_Jets.begin(), m_Jets.end(), std::greater<McObj>());
  ATH_MSG_DEBUG("  Summary.  "
                << " Number of jets found   = " << m_Jets.size() << " \n ");
  if (m_Jets.size() > 0) {
    ATH_MSG_DEBUG(" Highest pt (in GeV)  "
                  << (m_Jets[0].P().perp() / Gaudi::Units::GeV)
                  << "   Rapidity " << m_Jets[0].P().pseudoRapidity()
                  << "   Phi " << m_Jets[0].P().phi() << "\n ");
    ATH_MSG_DEBUG(" Second Highest pt (in GeV)  "
                  << (m_Jets[1].P().perp() / Gaudi::Units::GeV)
                  << "   Rapidity " << m_Jets[1].P().pseudoRapidity()
                  << "   Phi " << m_Jets[1].P().phi() << "\n ");
    ATH_MSG_DEBUG(" Lowest pt (in GeV)  "
                  << (m_Jets[m_Jets.size() - 1].P().perp() / Gaudi::Units::GeV)
                  << "   Rapidity "
                  << m_Jets[m_Jets.size() - 1].P().pseudoRapidity() << "   Phi "
                  << m_Jets[m_Jets.size() - 1].P().phi() << "\n ");
  }
  // now look at the jets and check the filter
  if (m_Jets.size() >= (unsigned)m_userNumber) {
    if (m_Jets[m_userNumber - 1].P().perp() > m_userThresh) {
      m_Jets.clear();  // clean it out
      return StatusCode::SUCCESS;
    }
  }
  setFilterPassed(false);  // it failed to find any useful jets
  m_Jets.clear();          // clean out the found jets
  return StatusCode::SUCCESS;
}
