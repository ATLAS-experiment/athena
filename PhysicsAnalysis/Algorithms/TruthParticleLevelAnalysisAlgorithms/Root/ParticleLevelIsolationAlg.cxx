/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>

#include "TruthParticleLevelAnalysisAlgorithms/ParticleLevelIsolationAlg.h"

#include <AsgDataHandles/ReadDecorHandle.h>
#include <AsgDataHandles/ReadHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>

#include <algorithm>
#include <optional>

namespace CP {

StatusCode ParticleLevelIsolationAlg::initialize() {

  ANA_CHECK(m_particlesKey.initialize());

  if (m_isolated.value().empty()) {
    ANA_MSG_ERROR("isolation decoration name is empty!");
    return StatusCode::FAILURE;
  }
  if (m_notTauOrigin.value().empty()) {
    ANA_MSG_ERROR("notTauOrigin decoration name is empty!");
    return StatusCode::FAILURE;
  }

  // decorators
  if (!m_decIsolatedKey.key().empty()) {
    ANA_MSG_WARNING("isolatedDecorKey is set internally and will be overwritten");
  }
  m_decIsolatedKey = m_particlesKey.key() + "." + m_isolated.value();
  ANA_CHECK(m_decIsolatedKey.initialize());
  if (!m_decNotTauOriginKey.key().empty()) {
    ANA_MSG_WARNING("notTauOriginDecorKey is set internally and will be overwritten");
  }
  m_decNotTauOriginKey = m_particlesKey.key() + "." + m_notTauOrigin.value();
  ANA_CHECK(m_decNotTauOriginKey.initialize());

  // accessors
  if (!m_isolationVariable.value().empty()) {
    if (!m_isoVarKey.key().empty()) {
      ANA_MSG_WARNING("isoVarDecorKey is set internally and will be overwritten");
    }
    m_isoVarKey = m_particlesKey.key() + "." + m_isolationVariable.value();
  }
  ANA_CHECK(m_isoVarKey.initialize(!m_isolationVariable.value().empty()));

  ANA_CHECK(m_classifierTypeKey.initialize());
  ANA_CHECK(m_classifierOriginKey.initialize());

  // set up MCTruthClassifier comparisons
  MCTruthPartClassifier::ParticleDef partDef;
  const auto it =
      std::ranges::find(partDef.sParticleType, m_checkTypeName.value());
  if (it == partDef.sParticleType.end()) {
    ANA_MSG_ERROR(
        "checkType = "
        << m_checkTypeName.value()
        << " is not a valid MCTruthPartClassifier::ParticleType string!");
    return StatusCode::FAILURE;
  } else {
    m_checkType = static_cast<MCTruthPartClassifier::ParticleType>(
        std::distance(partDef.sParticleType.begin(), it));
  }

  return StatusCode::SUCCESS;
}

StatusCode ParticleLevelIsolationAlg::execute(const EventContext &ctx) const {

  SG::ReadHandle<xAOD::TruthParticleContainer> particles(m_particlesKey, ctx);

  // accessors
  SG::ReadDecorHandle<xAOD::TruthParticleContainer, unsigned int> acc_type(
      m_classifierTypeKey, ctx);
  SG::ReadDecorHandle<xAOD::TruthParticleContainer, unsigned int> acc_orig(
      m_classifierOriginKey, ctx);
  std::optional<SG::ReadDecorHandle<xAOD::TruthParticleContainer, float>>
      acc_isoVar;
  if (!m_isoVarKey.empty())
    acc_isoVar.emplace(m_isoVarKey, ctx);

  // decoration availability is a property of the whole container
  const bool hasType = acc_type.isAvailable();
  const bool hasOrig = acc_orig.isAvailable();
  const bool hasIsoVar = acc_isoVar && acc_isoVar->isAvailable();

  // decorators
  SG::WriteDecorHandle<xAOD::TruthParticleContainer, char> dec_isolated(
      m_decIsolatedKey, ctx);
  SG::WriteDecorHandle<xAOD::TruthParticleContainer, char> dec_notTauOrigin(
      m_decNotTauOriginKey, ctx);

  for (const auto* particle : *particles) {

    // check the particle is isolated
    if (hasType) {
      bool isolation = acc_type(*particle) == m_checkType;
      // check further custom isolation cuts
      if (acc_isoVar && isolation) {
        if (hasIsoVar) {
          // treat particles without a valid pT as not isolated
          isolation = particle->pt() > 0 &&
                      (*acc_isoVar)(*particle) / particle->pt() <
                          m_isolationCut.value();
        } else {
          ANA_MSG_ERROR("Truth particle is missing the decoration: "
                        << m_isolationVariable.value() << ".");
          return StatusCode::FAILURE;
        }
      }
      dec_isolated(*particle) = isolation;
    } else {
      ANA_MSG_ERROR(
          "Truth particle is missing the decoration: classifierParticleType.");
      return StatusCode::FAILURE;
    }

    // check the particle doesn't come from a tau decay
    if (hasOrig) {
      dec_notTauOrigin(*particle) =
          acc_orig(*particle) != MCTruthPartClassifier::ParticleOrigin::TauLep;
    } else {
      ANA_MSG_ERROR(
          "Truth particle is missing the decoration: "
          "classifierParticleOrigin.");
      return StatusCode::FAILURE;
    }
  }
  return StatusCode::SUCCESS;
}

}  // namespace CP
