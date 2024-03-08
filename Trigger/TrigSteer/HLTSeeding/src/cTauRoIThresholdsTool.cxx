/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#include "cTauRoIThresholdsTool.h"
#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/exceptions.h"
#include "GaudiKernel/ThreadLocalContext.h"
#include "L1TopoAlgorithms/cTauMultiplicity.h"
#include "L1TopoCoreSim/TopoSteeringStructure.h"


StatusCode cTauRoIThresholdsTool::initialize() {
  ATH_CHECK(RoIThresholdsTool::initialize());
  ATH_CHECK(m_jTauLinkKey.initialize());
  return StatusCode::SUCCESS;
}

uint64_t cTauRoIThresholdsTool::getPattern(const xAOD::eFexTauRoI& eTau,
                                           const RoIThresholdsTool::ThrVec& menuThresholds,
                                           const TrigConf::L1ThrExtraInfoBase& menuExtraInfo) const {

  std::map<std::string, int> isoFW_CTAU, isoFW_CTAU_jTAUCoreScale;
  TCS::TopoSteeringStructure::setIsolationFW_CTAU(isoFW_CTAU, isoFW_CTAU_jTAUCoreScale, menuExtraInfo);

  unsigned int eFexEt{eTau.etTOB()}; // eTAU Et in units of 100 MeV
  int eFexEta{eTau.iEta()};

  unsigned int jFexCoreEt = 0, jFexIsoEt = 0; // jTAU core and isolation Et in units of 200 MeV

  // Get the jTau matched to the eTau
  using jTauLink_t = ElementLink<xAOD::jFexTauRoIContainer>;
  SG::ReadDecorHandle<xAOD::eFexTauRoIContainer, jTauLink_t> jTauLinkAcc{m_jTauLinkKey, Gaudi::Hive::currentContext()};
  if (not jTauLinkAcc.isPresent()) {
    ATH_MSG_ERROR("Decoration " << m_jTauLinkKey.key() << " is missing, cannot create cTau threshold pattern");
    throw SG::ExcNullReadHandle(m_jTauLinkKey.clid(), m_jTauLinkKey.key(), m_jTauLinkKey.storeHandle().name());
  }
  jTauLink_t jTauLink = jTauLinkAcc(eTau);
  bool matched{jTauLink.isValid()};

  if (matched) {
    const xAOD::jFexTauRoI* jTau = *jTauLink;

    jFexIsoEt = jTau->tobIso();
    jFexCoreEt = jTau->tobEt();

    ATH_MSG_DEBUG("eFex tau eta,phi = " << eTau.iEta() << ", " << eTau.iPhi()
                  << ", jFex tau eta,phi = " << jTau->globalEta() << ", " << jTau->globalPhi()
                  << ", eFex et (100 MeV/counts) = " << eFexEt << ", jFex et (200 MeV/counts) = " << jTau->tobEt() << ", jFex iso (200 MeV/counts) = " << jTau->tobIso());
  } else {
    ATH_MSG_DEBUG("eFex tau eta,phi = " << eTau.iEta() << ", " << eTau.iPhi()
                  << ", eFex et (100 MeV/counts) = " << eFexEt << ", no matching jTau found");
  }


  uint64_t thresholdMask{0};

  // Iterate through thresholds and see which ones are passed
  for (const std::shared_ptr<TrigConf::L1Threshold>& thrBase : menuThresholds) {
    std::shared_ptr<TrigConf::L1Threshold_cTAU> thr = std::static_pointer_cast<TrigConf::L1Threshold_cTAU>(thrBase);

    // Check isolation threshold - unmatched eTau treated as perfectly isolated, ATR-25927
    // The core and isolation E_T values are multiplied by 2 to normalise to 100 MeV/counts units
    bool passIso = matched ? TCS::cTauMultiplicity::checkIsolationWP(isoFW_CTAU, isoFW_CTAU_jTAUCoreScale, 2*static_cast<float>(jFexCoreEt), 2*static_cast<float>(jFexIsoEt), static_cast<float>(eFexEt), TrigConf::Selection::wpToString(thr->isolation())) : true;

    // Check pt threshold - using iEta coordinate for the eFEX ensures a 0.1 granularity of the eta coordinate,
    // as expected from the menu method thrValue100MeV
    bool passPt = eFexEt > thr->thrValue100MeV(eFexEta);

    if ( passIso && passPt ) {
      thresholdMask |= (1<<thr->mapping());
    }

  } // loop over thr

  return thresholdMask;
}
