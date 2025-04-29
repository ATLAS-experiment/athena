/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Christian Grefe



//
// includes
//

#include <TauAnalysisAlgorithms/TauTruthDecorationsAlg.h>

#include <AsgDataHandles/ReadHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>
#include <AthContainers/AuxElement.h>
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

    for (const auto& decorationName : m_doubleDecorations) {
      auto [it, added] = m_doubleWriteHandleKeys.emplace(std::make_unique<SG::AuxElement::ConstAccessor<double>>(decorationName), SG::WriteDecorHandleKey<xAOD::TauJetContainer>(m_tausKey.key() + "." + m_prefix + decorationName));
      ANA_CHECK(it->second.initialize());
    }
    for (const auto& decorationName : m_floatDecorations) {
      auto [it, added] = m_floatWriteHandleKeys.emplace(std::make_unique<SG::AuxElement::ConstAccessor<float>>(decorationName), SG::WriteDecorHandleKey<xAOD::TauJetContainer>(m_tausKey.key() + "." + m_prefix + decorationName));
      ANA_CHECK(it->second.initialize());
    }
    for (const auto& decorationName : m_intDecorations) {
      auto [it, added] = m_intWriteHandleKeys.emplace(std::make_unique<SG::AuxElement::ConstAccessor<int>>(decorationName), SG::WriteDecorHandleKey<xAOD::TauJetContainer>(m_tausKey.key() + "." + m_prefix + decorationName));
      ANA_CHECK(it->second.initialize());
    }
    for (const auto& decorationName : m_unsignedIntDecorations) {
      auto [it, added] = m_unsignedIntWriteHandleKeys.emplace(std::make_unique<SG::AuxElement::ConstAccessor<unsigned int>>(decorationName), SG::WriteDecorHandleKey<xAOD::TauJetContainer>(m_tausKey.key() + "." + m_prefix + decorationName));
      ANA_CHECK(it->second.initialize());
    }
    for (const auto& decorationName : m_charDecorations) {
      auto [it, added] = m_charWriteHandleKeys.emplace(std::make_unique<SG::AuxElement::ConstAccessor<char>>(decorationName), SG::WriteDecorHandleKey<xAOD::TauJetContainer>(m_tausKey.key() + "." + m_prefix + decorationName));
      ANA_CHECK(it->second.initialize());
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

    ANA_CHECK(m_truthDecayModeKey.initialize());
    ANA_CHECK(m_truthParticleTypeKey.initialize());
    ANA_CHECK(m_partonTruthLabelIDKey.initialize());

    return StatusCode::SUCCESS;
  }



  StatusCode TauTruthDecorationsAlg ::
  execute (const EventContext &ctx) const
  {
    SG::ReadHandle<xAOD::TauJetContainer> taus(m_tausKey, ctx);

    std::unordered_map<SG::AuxElement::ConstAccessor<double> *, SG::WriteDecorHandle<xAOD::TauJetContainer, float>> doubleWriteHandles;
    std::unordered_map<SG::AuxElement::ConstAccessor<float> *, SG::WriteDecorHandle<xAOD::TauJetContainer, float>> floatWriteHandles;
    std::unordered_map<SG::AuxElement::ConstAccessor<int> *, SG::WriteDecorHandle<xAOD::TauJetContainer, int>> intWriteHandles;
    std::unordered_map<SG::AuxElement::ConstAccessor<unsigned int> *, SG::WriteDecorHandle<xAOD::TauJetContainer, unsigned int>> unsignedIntWriteHandles;
    std::unordered_map<SG::AuxElement::ConstAccessor<char> *, SG::WriteDecorHandle<xAOD::TauJetContainer, char>> charWriteHandles;
    for (auto &[acc, writeHandleKey] : m_doubleWriteHandleKeys) {
      doubleWriteHandles.emplace(acc.get(), SG::WriteDecorHandle<xAOD::TauJetContainer, float>(writeHandleKey, ctx));
    }
    for (auto &[acc, writeHandleKey] : m_floatWriteHandleKeys) {
      floatWriteHandles.emplace(acc.get(), SG::WriteDecorHandle<xAOD::TauJetContainer, float>(writeHandleKey, ctx));
    }
    for (auto &[acc, writeHandleKey] : m_intWriteHandleKeys) {
      intWriteHandles.emplace(acc.get(), SG::WriteDecorHandle<xAOD::TauJetContainer, int>(writeHandleKey, ctx));
    }
    for (auto &[acc, writeHandleKey] : m_unsignedIntWriteHandleKeys) {
      unsignedIntWriteHandles.emplace(acc.get(), SG::WriteDecorHandle<xAOD::TauJetContainer, unsigned int>(writeHandleKey, ctx));
    }
    for (auto &[acc, writeHandleKey] : m_charWriteHandleKeys) {
      charWriteHandles.emplace(acc.get(), SG::WriteDecorHandle<xAOD::TauJetContainer, char>(writeHandleKey, ctx));
    }
    
    SG::WriteDecorHandle<xAOD::TauJetContainer, float> truthDecayModeHandle(m_truthDecayModeKey, ctx);
    SG::WriteDecorHandle<xAOD::TauJetContainer, float> truthParticleTypeHandle(m_truthParticleTypeKey, ctx);
    SG::WriteDecorHandle<xAOD::TauJetContainer, float> partonTruthLabelIDHandle(m_partonTruthLabelIDKey, ctx);
    
    //
    for (const xAOD::TauJet *tau : *taus){
      const xAOD::TruthParticle* truthParticle = xAOD::TauHelpers::getTruthParticle(tau);
      //ensure _each tau_ is decorated with something, even if truthParticle is nullptr
      //
      //some cruft to get from the unordered map to the underlying  type of the handles
      //T::mapped_type::accessor_t::element_type will be int, float, char etc
      auto decorateWithNumber = [&truthParticle, &tau]<typename T>(T & writeHandles, T::mapped_type::accessor_t::element_type v)->void{
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

      static const SG::AuxElement::ConstAccessor<int> acc_PartonTruthLabelID("PartonTruthLabelID");
      const xAOD::Jet *truthJet = xAOD::TauHelpers::getLink<xAOD::Jet>(tau, "truthJetLink");
      if (truthJet != nullptr) {
        partonTruthLabelIDHandle(*tau) = acc_PartonTruthLabelID(*truthJet);
      }
    }

    return StatusCode::SUCCESS;
  }
}
