
/*
Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#include "GeneratorFilters/xAODFourLeptonMassFilter.h"

#include "GaudiKernel/PhysicalConstants.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthParticleAuxContainer.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "TruthUtils/HepMCHelpers.h"

xAODFourLeptonMassFilter::xAODFourLeptonMassFilter(const std::string& name,
                                                   ISvcLocator* pSvcLocator)
    : GenFilter(name, pSvcLocator) {}
StatusCode xAODFourLeptonMassFilter::filterInitialize() {
  ATH_MSG_DEBUG("MinPt " << m_minPt);
  ATH_MSG_DEBUG("MaxEta " << m_maxEta);
  ATH_MSG_DEBUG("MinMass1 " << m_minMass1);
  ATH_MSG_DEBUG("MaxMass1 " << m_maxMass1);
  ATH_MSG_DEBUG("MinMass2 " << m_minMass2);
  ATH_MSG_DEBUG("MaxMass2 " << m_maxMass2);
  ATH_MSG_DEBUG("AllowElecMu " << m_allowElecMu);
  ATH_MSG_DEBUG("AllowSameCharge " << m_allowSameCharge);
  ATH_CHECK(m_xaodTruthParticleContainerNameLightLeptonKey.initialize());
  return StatusCode::SUCCESS;
}
StatusCode xAODFourLeptonMassFilter::filterEvent(const EventContext& ctx) {
  // Retrieve TruthLightLepton container from xAOD LightLepton slimmer, contains
  // (electrons and muons ) particles

  SG::ReadHandle<xAOD::TruthParticleContainer>
      xTruthParticleContainerReadHandle(
          m_xaodTruthParticleContainerNameLightLeptonKey, ctx);
  if (!xTruthParticleContainerReadHandle.isValid()) {
    ATH_MSG_ERROR("Could not retrieve xAOD::TruthParticleContainer with key:"
                  << m_xaodTruthParticleContainerNameLightLeptonKey.key());

    return StatusCode::FAILURE;
  }

  unsigned int nParticles = xTruthParticleContainerReadHandle->size();
  // loop over all particles
  for (unsigned int iPart = 0; iPart < nParticles; ++iPart) {
    const xAOD::TruthParticle* lightLeptonParticle =
        (*xTruthParticleContainerReadHandle)[iPart];
    int pdgId1 = lightLeptonParticle->pdgId();
    if (!MC::isStable(lightLeptonParticle))
      continue;
    // Pick electrons or muons with Pt > m_inPt and |eta| < m_maxEta
    if (!(lightLeptonParticle->isElectron() || lightLeptonParticle->isMuon()))
      continue;
    if (lightLeptonParticle->pt() < m_minPt ||
        lightLeptonParticle->abseta() > m_maxEta)

      continue;
    // loop over all remaining particles in the event
    for (unsigned int iPart2 = iPart + 1; iPart2 < nParticles; ++iPart2) {
      const xAOD::TruthParticle* lightLeptonParticle2 =
          (*xTruthParticleContainerReadHandle)[iPart2];
      int pdgId2 = lightLeptonParticle2->pdgId();
      if ( !MC::isStable(lightLeptonParticle) || iPart == iPart2)
        continue;
      // Pick electrons or muons with Pt > m_inPt and |eta| < m_maxEta
      if (!(lightLeptonParticle2->isElectron() ||
            lightLeptonParticle2->isMuon()))
        continue;
      if (lightLeptonParticle2->pt() < m_minPt ||
          lightLeptonParticle2->abseta() > m_maxEta)
        continue;
      // Loop over all remaining particles in the event
      for (unsigned int iPart3 = iPart2 + 1; iPart3 < nParticles; ++iPart3) {
        const xAOD::TruthParticle* lightLeptonParticle3 =
            (*xTruthParticleContainerReadHandle)[iPart3];
        int pdgId3 = lightLeptonParticle3->pdgId();
        if (!MC::isStable(lightLeptonParticle) || iPart == iPart3 ||
            iPart2 == iPart3)
          continue;
        // Pick electrons or muons with Pt > m_inPt and |eta| < m_maxEta
        if (!(lightLeptonParticle3->isElectron() ||
              lightLeptonParticle3->isMuon()))
          continue;
        if (lightLeptonParticle3->pt() < m_minPt ||
            lightLeptonParticle3->abseta() > m_maxEta)
          continue;
        // Loop over all remaining particles in the event
        for (unsigned int iPart4 = iPart3 + 1; iPart4 < nParticles; ++iPart4) {
          const xAOD::TruthParticle* lightLeptonParticle4 =
              (*xTruthParticleContainerReadHandle)[iPart4];
          int pdgId4 = lightLeptonParticle4->pdgId();
          if (!MC::isStable(lightLeptonParticle) || iPart == iPart4 ||
              iPart2 == iPart4 || iPart3 == iPart4)
            continue;
          // Pick electrons or muons with Pt > m_inPt and |eta| < m_maxEta
          if (!(lightLeptonParticle4->isElectron() ||
                lightLeptonParticle4->isMuon()))
            continue;
          if (lightLeptonParticle4->pt() < m_minPt ||
              lightLeptonParticle4->abseta() > m_maxEta)
            continue;
          decltype(lightLeptonParticle) apitr[4] = {
              lightLeptonParticle, lightLeptonParticle2, lightLeptonParticle3,
              lightLeptonParticle4};
          int pdgIds[4] = {pdgId1, pdgId2, pdgId3, pdgId4};
          for (int ii = 0; ii < 4; ii++) {
            for (int jj = 0; jj < 4; jj++) {
              if (jj == ii)
                continue;
              for (int kk = 0; kk < 4; kk++) {
                if (kk == jj || kk == ii)
                  continue;
                for (int ll = 0; ll < 4; ll++) {
                  if (ll == kk || ll == jj || ll == ii)
                    continue;
                  if (!m_allowElecMu &&
                      (std::abs(pdgIds[ii]) != std::abs(pdgIds[jj]) ||
                       std::abs(pdgIds[kk]) != std::abs(pdgIds[ll])))
                    continue;
                  if (!m_allowSameCharge && (pdgIds[ii] * pdgIds[jj] > 0. ||
                                             pdgIds[kk] * pdgIds[ll] > 0.))
                    continue;
                  // Leading dilepton pair
                  HepMC::FourVector vec(
                      ((apitr[ii]))->px() + ((apitr[jj]))->px(),
                      ((apitr[ii]))->py() + ((apitr[jj]))->py(),
                      ((apitr[ii]))->pz() + ((apitr[jj]))->pz(),
                      ((apitr[ii]))->e() + ((apitr[jj]))->e());
                  double invMass1 = vec.m();
                  if (invMass1 < m_minMass1 || invMass1 > m_maxMass1)
                    continue;
                  ATH_MSG_DEBUG("PASSED FILTER1 " << invMass1);
                  // Sub-leading dilepton pair
                  HepMC::FourVector vec2(
                      ((apitr[kk]))->px() + ((apitr[ll]))->px(),
                      ((apitr[kk]))->py() + ((apitr[ll]))->py(),
                      ((apitr[kk]))->pz() + ((apitr[ll]))->pz(),
                      ((apitr[kk]))->e() + ((apitr[ll]))->e());
                  double invMass2 = vec2.m();
                  if (invMass2 < m_minMass2 || invMass2 > m_maxMass2)
                    continue;
                  ATH_MSG_DEBUG("PASSED FILTER2 " << invMass2);
                  return StatusCode::SUCCESS;
                }
              }
            }
          }
        }
      }
    }
  }
  setFilterPassed(false, ctx);
  return StatusCode::SUCCESS;
}
