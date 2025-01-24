/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>

#include "TruthParticleLevelAnalysisAlgorithms/ParticleLevelIsolationAlg.h"

namespace CP {

StatusCode ParticleLevelIsolationAlg::initialize() {

  ANA_CHECK(m_particlesKey.initialize());

  // decorators
  m_dec_isolated =
      std::make_unique<SG::AuxElement::Decorator<char>>(m_isolated.value());
  m_dec_notTauOrigin =
      std::make_unique<SG::AuxElement::Decorator<char>>(m_notTauOrigin.value());

  // accessors
  if (m_isolationVariable.value() != "") {
    m_acc_isoVar = std::make_unique<SG::AuxElement::ConstAccessor<float>>(
        m_isolationVariable.value());
  }

  // set up MCTruthClassifier comparisons
  MCTruthPartClassifier::ParticleDef partDef;
  const auto it =
      std::find(partDef.sParticleType.begin(), partDef.sParticleType.end(),
                m_checkTypeName.value());
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
  static const SG::AuxElement::ConstAccessor<unsigned int> acc_type(
      "classifierParticleType");
  static const SG::AuxElement::ConstAccessor<unsigned int> acc_orig(
      "classifierParticleOrigin");

  // decorators
  static const SG::AuxElement::Decorator<float> dec_charge("charge");

  for (const auto* particle : *particles) {

    // check the particle is isolated
    if (acc_type.isAvailable(*particle)) {
      bool isolation = acc_type(*particle) == m_checkType;
      // check further custom isolation cuts
      if (m_acc_isoVar && isolation) {
        if ((*m_acc_isoVar).isAvailable(*particle)) {
          isolation =
              isolation && ((*m_acc_isoVar)(*particle) / particle->pt() <
                            m_isolationCut.value());
        } else {
          ANA_MSG_ERROR("Truth particle is missing the decoration: "
                        << m_isolationVariable.value() << ".");
          return StatusCode::FAILURE;
        }
      }
      (*m_dec_isolated)(*particle) = isolation;
    } else {
      ANA_MSG_ERROR(
          "Truth particle is missing the decoration: classifierParticleType.");
      return StatusCode::FAILURE;
    }

    // check the particle doesn't come from a tau decay
    if (acc_orig.isAvailable(*particle)) {
      (*m_dec_notTauOrigin)(*particle) =
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
