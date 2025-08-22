/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "cTauRoIThresholdsTool.h"
#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/exceptions.h"
#include "GaudiKernel/ThreadLocalContext.h"
#include "L1TopoAlgorithms/cTauMultiplicity.h"


StatusCode cTauRoIThresholdsTool::initialize() {
  ATH_CHECK(RoIThresholdsTool::initialize());
  ATH_CHECK(m_jTauLinkKey.initialize());
  return StatusCode::SUCCESS;
}

uint64_t cTauRoIThresholdsTool::getPattern(const EventContext& ctx,
                                           const xAOD::eFexTauRoI& eTau,
                                           const RoIThresholdsTool::ThrVec& menuThresholds,
                                           const TrigConf::L1ThrExtraInfoBase& /*menuExtraInfo*/) const {

  // Get the jTau matched to the eTau
  using jTauLink_t = ElementLink<xAOD::jFexTauRoIContainer>;
  SG::ReadDecorHandle<xAOD::eFexTauRoIContainer, jTauLink_t> jTauLinkAcc{m_jTauLinkKey, ctx};
  if (not jTauLinkAcc.isPresent()) {
    ATH_MSG_ERROR("Decoration " << m_jTauLinkKey.key() << " is missing, cannot create cTau threshold pattern");
    throw SG::ExcNullReadHandle(m_jTauLinkKey.clid(), m_jTauLinkKey.key(), m_jTauLinkKey.storeHandle().name());
  }
  jTauLink_t jTauLink = jTauLinkAcc(eTau);
  bool matched{jTauLink.isValid()};

  if (matched) {
    const xAOD::jFexTauRoI* jTau = *jTauLink;

    ATH_MSG_DEBUG("eFex tau eta,phi = " << eTau.iEta() << ", " << eTau.iPhi()
                  << ", jFex tau eta,phi = " << jTau->globalEta() << ", " << jTau->globalPhi()
                  << ", eFex et (100 MeV/counts) = " << eTau.etTOB() << ", jFex et (200 MeV/counts) = " << jTau->tobEt() << ", jFex iso (200 MeV/counts) = " << jTau->tobIso()
                  << ", eFex rCore/BDT = " << eTau.tauOneThresholds() << ", eFex rHad = " << eTau.tauTwoThresholds());
  } else {
    ATH_MSG_DEBUG("eFex tau eta,phi = " << eTau.iEta() << ", " << eTau.iPhi()
                  << ", eFex et (100 MeV/counts) = " << eTau.etTOB() << ", no matching jTau found"
                  << ", eFex rCore/BDT = " << eTau.tauOneThresholds() << ", eFex rHad = " << eTau.tauTwoThresholds());
  }


  uint64_t thresholdMask{0};

  // Iterate through thresholds and see which ones are passed
  for (const std::shared_ptr<TrigConf::L1Threshold>& thrBase : menuThresholds) {
    std::shared_ptr<TrigConf::L1Threshold_cTAU> thr = std::static_pointer_cast<TrigConf::L1Threshold_cTAU>(thrBase);

    // Check isolation threshold - unmatched eTau treated as perfectly isolated, ATR-25927
    // The core and isolation E_T values are multiplied by 2 to normalise to 100 MeV/counts units
    bool passIso = matched ? TCS::cTauMultiplicity::checkIsolationWP(eTau, **jTauLink, *thr) : true;

    // Check eTAU rCore/BDT and rHad thresholds
    bool passeTAUWP = TCS::cTauMultiplicity::checkeTAUWP(eTau, *thr);

    // Check et threshold - using iEta coordinate for the eFEX ensures a 0.1 granularity of the eta coordinate,
    // as expected from the menu method thrValue100MeV
    bool passEt = eTau.etTOB() > thr->thrValue100MeV(eTau.iEta());

    if (passIso && passeTAUWP && passEt) {
      thresholdMask |= (1<<thr->mapping());
    }

  } // loop over thr

  return thresholdMask;
}
