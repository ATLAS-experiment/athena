/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file CachedGetAssocTruth.cxx
 * @author shaun roe
 * @date 11 October 2016
 **/

 #include "CachedGetAssocTruth.h"
 #include "xAODTruth/TruthParticleContainer.h"
 #include "AthContainers/ConstAccessor.h"

namespace IDPVM {
  void
  CachedGetAssocTruth::clear() {
    m_cache.clear();
  }

  const xAOD::TruthParticle*
  CachedGetAssocTruth::getTruth(const xAOD::TrackParticle* trackParticle) {
    if (not trackParticle) {
      return nullptr;
    }
    auto pCache = m_cache.find(trackParticle);
    if (pCache != m_cache.end()) {
      return pCache->second;
    }
    using ElementTruthLink_t = ElementLink<xAOD::TruthParticleContainer>;
    const xAOD::TruthParticle* result(nullptr);
    static const SG::ConstAccessor<ElementTruthLink_t> truthParticleLinkAcc("truthParticleLink");
    // 0. is there any truth?
    if (truthParticleLinkAcc.isAvailable(*trackParticle)) {
      // 1. ..then get link
      const ElementTruthLink_t ptruthContainer = truthParticleLinkAcc(*trackParticle);
      if (ptruthContainer.isValid()) {
        result = *ptruthContainer;
      }
    }
    m_cache[trackParticle] = result;
    return result;
  }

  const xAOD::TruthParticle*
  CachedGetAssocTruth::operator () (const xAOD::TrackParticle* trackParticle) {
    return getTruth(trackParticle);
  }

}
