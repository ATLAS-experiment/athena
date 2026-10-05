/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Christian Grefe



//
// includes
//

#include <TauAnalysisAlgorithms/TauTruthDecorationsAlg.h>

#include <AsgDataHandles/ReadHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>
#include <xAODTruth/TruthParticleContainer.h>
#include <TauAnalysisTools/HelperFunctions.h>

//
// method implementations
//



namespace CP
{

  StatusCode TauTruthDecorationsAlg ::
  initialize ()
  {
    ANA_CHECK(m_tausKey.initialize());

    m_doubleWriteHandleKeys.reserve(m_doubleDecorations.size());
    for (const auto& decorationName : m_doubleDecorations) {
      auto& key = m_doubleWriteHandleKeys.emplace_back(SG::ConstAccessor<double>(decorationName), SG::WriteDecorHandleKey<xAOD::TauJetContainer>(m_tausKey.key() + "." + m_prefix + decorationName)).second;
      ANA_CHECK(key.initialize());
      // register output decoration type for output algorithms
      // note that even though we read a double we write it as a float
      SG::ConstAccessor<float> (key.key().substr (key.key().find_last_of(".") + 1));
    }
    m_floatWriteHandleKeys.reserve(m_floatDecorations.size());
    for (const auto& decorationName : m_floatDecorations) {
      auto& key = m_floatWriteHandleKeys.emplace_back(SG::ConstAccessor<float>(decorationName), SG::WriteDecorHandleKey<xAOD::TauJetContainer>(m_tausKey.key() + "." + m_prefix + decorationName)).second;
      ANA_CHECK(key.initialize());
      // register output decoration type for output algorithms
      SG::ConstAccessor<float> (key.key().substr (key.key().find_last_of(".") + 1));
    }
    m_intWriteHandleKeys.reserve(m_intDecorations.size());
    for (const auto& decorationName : m_intDecorations) {
      auto& key = m_intWriteHandleKeys.emplace_back(SG::ConstAccessor<int>(decorationName), SG::WriteDecorHandleKey<xAOD::TauJetContainer>(m_tausKey.key() + "." + m_prefix + decorationName)).second;
      ANA_CHECK(key.initialize());
      // register output decoration type for output algorithms
      SG::ConstAccessor<int> (key.key().substr (key.key().find_last_of(".") + 1));
    }
    m_unsignedIntWriteHandleKeys.reserve(m_unsignedIntDecorations.size());
    for (const auto& decorationName : m_unsignedIntDecorations) {
      auto& key = m_unsignedIntWriteHandleKeys.emplace_back(SG::ConstAccessor<unsigned int>(decorationName), SG::WriteDecorHandleKey<xAOD::TauJetContainer>(m_tausKey.key() + "." + m_prefix + decorationName)).second;
      ANA_CHECK(key.initialize());
      // register output decoration type for output algorithms
      SG::ConstAccessor<unsigned int> (key.key().substr (key.key().find_last_of(".") + 1));
    }
    m_charWriteHandleKeys.reserve(m_charDecorations.size());
    for (const auto& decorationName : m_charDecorations) {
      auto& key = m_charWriteHandleKeys.emplace_back(SG::ConstAccessor<char>(decorationName), SG::WriteDecorHandleKey<xAOD::TauJetContainer>(m_tausKey.key() + "." + m_prefix + decorationName)).second;
      ANA_CHECK(key.initialize());
      // register output decoration type for output algorithms
      SG::ConstAccessor<char> (key.key().substr (key.key().find_last_of(".") + 1));
    }

    if (m_truthDecayModeKey.contHandleKey().key() == m_truthDecayModeKey.key()) {
      m_truthDecayModeKey = m_tausKey.key() + "." + m_truthDecayModeKey.key();
    }
    if (m_truthParticleTypeKey.contHandleKey().key() == m_truthParticleTypeKey.key()) {
      m_truthParticleTypeKey = m_tausKey.key() + "." + m_truthParticleTypeKey.key();
    }
    if (m_partonTruthLabelIDKey.contHandleKey().key() == m_partonTruthLabelIDKey.key()) {
      m_partonTruthLabelIDKey = m_tausKey.key() + "." + m_partonTruthLabelIDKey.key();
    }
    ANA_CHECK(m_truthDecayModeKey.initialize());
    ANA_CHECK(m_truthParticleTypeKey.initialize());
    ANA_CHECK(m_partonTruthLabelIDKey.initialize());

    // register output decoration type for output algorithms
    SG::ConstAccessor<int> (m_truthDecayModeKey.key().substr (m_truthDecayModeKey.key().find_last_of(".") + 1));
    SG::ConstAccessor<int> (m_truthParticleTypeKey.key().substr (m_truthParticleTypeKey.key().find_last_of(".") + 1));
    SG::ConstAccessor<int> (m_partonTruthLabelIDKey.key().substr (m_partonTruthLabelIDKey.key().find_last_of(".") + 1));

    return StatusCode::SUCCESS;
  }



  StatusCode TauTruthDecorationsAlg ::
  execute (const EventContext &ctx) const
  {
    SG::ReadHandle<xAOD::TauJetContainer> taus(m_tausKey, ctx);

    std::vector<std::pair<const SG::ConstAccessor<double> *, SG::WriteDecorHandle<xAOD::TauJetContainer, float>>> doubleWriteHandles;
    std::vector<std::pair<const SG::ConstAccessor<float> *, SG::WriteDecorHandle<xAOD::TauJetContainer, float>>> floatWriteHandles;
    std::vector<std::pair<const SG::ConstAccessor<int> *, SG::WriteDecorHandle<xAOD::TauJetContainer, int>>> intWriteHandles;
    std::vector<std::pair<const SG::ConstAccessor<unsigned int> *, SG::WriteDecorHandle<xAOD::TauJetContainer, unsigned int>>> unsignedIntWriteHandles;
    std::vector<std::pair<const SG::ConstAccessor<char> *, SG::WriteDecorHandle<xAOD::TauJetContainer, char>>> charWriteHandles;
    doubleWriteHandles.reserve(m_doubleWriteHandleKeys.size());
    for (const auto &[acc, writeHandleKey] : m_doubleWriteHandleKeys) {
      doubleWriteHandles.emplace_back(&acc, SG::WriteDecorHandle<xAOD::TauJetContainer, float>(writeHandleKey, ctx));
    }
    floatWriteHandles.reserve(m_floatWriteHandleKeys.size());
    for (const auto &[acc, writeHandleKey] : m_floatWriteHandleKeys) {
      floatWriteHandles.emplace_back(&acc, SG::WriteDecorHandle<xAOD::TauJetContainer, float>(writeHandleKey, ctx));
    }
    intWriteHandles.reserve(m_intWriteHandleKeys.size());
    for (const auto &[acc, writeHandleKey] : m_intWriteHandleKeys) {
      intWriteHandles.emplace_back(&acc, SG::WriteDecorHandle<xAOD::TauJetContainer, int>(writeHandleKey, ctx));
    }
    unsignedIntWriteHandles.reserve(m_unsignedIntWriteHandleKeys.size());
    for (const auto &[acc, writeHandleKey] : m_unsignedIntWriteHandleKeys) {
      unsignedIntWriteHandles.emplace_back(&acc, SG::WriteDecorHandle<xAOD::TauJetContainer, unsigned int>(writeHandleKey, ctx));
    }
    charWriteHandles.reserve(m_charWriteHandleKeys.size());
    for (const auto &[acc, writeHandleKey] : m_charWriteHandleKeys) {
      charWriteHandles.emplace_back(&acc, SG::WriteDecorHandle<xAOD::TauJetContainer, char>(writeHandleKey, ctx));
    }
    
    SG::WriteDecorHandle<xAOD::TauJetContainer, int> truthDecayModeHandle(m_truthDecayModeKey, ctx);
    SG::WriteDecorHandle<xAOD::TauJetContainer, int> truthParticleTypeHandle(m_truthParticleTypeKey, ctx);
    SG::WriteDecorHandle<xAOD::TauJetContainer, int> partonTruthLabelIDHandle(m_partonTruthLabelIDKey, ctx);
    
    //
    for (const xAOD::TauJet *tau : *taus){
      const xAOD::TruthParticle* truthParticle = xAOD::TauHelpers::getTruthParticle(tau);
      //ensure _each tau_ is decorated with something, even if truthParticle is nullptr
      auto decorateWithNumber = [&truthParticle, &tau](auto & writeHandles, auto v)->void{
        for (auto& [acc, writeHandle] : writeHandles) {
          if ((!truthParticle) or (!acc->isAvailable(*truthParticle)) ) {
            writeHandle(*tau) = v;
          } else {
            writeHandle(*tau) = (*acc)(*truthParticle);
          }
        }
      };
      decorateWithNumber(doubleWriteHandles, -999.f);
      decorateWithNumber(floatWriteHandles, -999.f);
      decorateWithNumber(intWriteHandles, 0);
      decorateWithNumber(unsignedIntWriteHandles, 0);
      decorateWithNumber(charWriteHandles, 0);

      truthDecayModeHandle(*tau) = truthParticle ? TauAnalysisTools::getTruthDecayMode(*truthParticle) : xAOD::TauJetParameters::Mode_Error;
      truthParticleTypeHandle(*tau) = static_cast<int>(TauAnalysisTools::getTruthParticleType(*tau));
      //coverity[UNNECESSARY_STRING_COPY:FALSE]
      static const SG::ConstAccessor<int> acc_PartonTruthLabelID("PartonTruthLabelID");
      const xAOD::Jet *truthJet = xAOD::TauHelpers::getLink<xAOD::Jet>(tau, "truthJetLink");
      if (truthJet != nullptr) {
        partonTruthLabelIDHandle(*tau) = acc_PartonTruthLabelID(*truthJet);
      } else {
        partonTruthLabelIDHandle(*tau) = -999;
      }
    }

    return StatusCode::SUCCESS;
  }
}
