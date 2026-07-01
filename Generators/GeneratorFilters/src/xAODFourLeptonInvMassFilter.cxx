/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "GeneratorFilters/xAODFourLeptonInvMassFilter.h"
#include "GaudiKernel/PhysicalConstants.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthParticleAuxContainer.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "TruthUtils/HepMCHelpers.h"
#include <math.h>


xAODFourLeptonInvMassFilter::xAODFourLeptonInvMassFilter(const std::string & name,
  ISvcLocator * pSvcLocator): GenFilter(name, pSvcLocator) {}
StatusCode xAODFourLeptonInvMassFilter::filterInitialize() {
  ATH_MSG_DEBUG("MinPt " << m_minPt);
  ATH_MSG_DEBUG("MaxEta " << m_maxEta);
  ATH_MSG_DEBUG("MinMass " << m_minMass);
  ATH_MSG_DEBUG("MaxMass " << m_maxMass);

  ATH_CHECK(m_xaodTruthParticleContainerNameLightLeptonKey.initialize());
  return StatusCode::SUCCESS;
}

// all lepton selection in one place
bool xAODFourLeptonInvMassFilter::passesLeptonSelection(
  const xAOD::TruthParticle * p) const {

  if (!MC::isStable(p)) return false;
  if (!(p -> isElectron() || p -> isMuon())) return false;
  if (p -> pt() < m_minPt) return false;
  if (p -> abseta() > m_maxEta) return false;

  return true;
}

StatusCode xAODFourLeptonInvMassFilter::filterEvent(const EventContext& ctx) {
  // Retrieve TruthLightLepton container from xAOD LightLepton slimmer, contains
  // (electrons and muons ) particles

  SG::ReadHandle < xAOD::TruthParticleContainer >
    xTruthParticleContainerReadHandle(
      m_xaodTruthParticleContainerNameLightLeptonKey, ctx);
  if (!xTruthParticleContainerReadHandle.isValid()) {
    ATH_MSG_ERROR("Could not retrieve xAOD::TruthParticleContainer with key:" <<
      m_xaodTruthParticleContainerNameLightLeptonKey.key());

    return StatusCode::FAILURE;

  }

  // Collect selected leptons
  std::vector <
    const xAOD::TruthParticle * > lightLeptonParticle;
  for (const xAOD::TruthParticle * p: * xTruthParticleContainerReadHandle)
    if (passesLeptonSelection(p))
      lightLeptonParticle.push_back(p);

  if (lightLeptonParticle.size() < 4) {
    setFilterPassed(false, ctx);
    return StatusCode::SUCCESS;
  }

  // Loop over all particles
  for (size_t iPart = 0; iPart < lightLeptonParticle.size(); ++iPart)
    for (size_t iPart2 = iPart + 1; iPart2 < lightLeptonParticle.size(); ++iPart2)
      for (size_t iPart3 = iPart2 + 1; iPart3 < lightLeptonParticle.size(); ++iPart3)
        for (size_t iPart4 = iPart3 + 1; iPart4 < lightLeptonParticle.size(); ++iPart4) {

          xAOD::TruthParticle::GenVecFourMom_t vec = lightLeptonParticle[iPart] -> p4() + lightLeptonParticle[iPart2] -> p4() +
            lightLeptonParticle[iPart3] -> p4() + lightLeptonParticle[iPart4] -> p4();

          double m = vec.M();
          if (m > m_minMass && m < m_maxMass) {
            ATH_MSG_DEBUG("PASSED FILTER: invariant mass = " << m);
            setFilterPassed(true, ctx);
            return StatusCode::SUCCESS;
          }

        }

  setFilterPassed(false, ctx);
  return StatusCode::SUCCESS;
}
