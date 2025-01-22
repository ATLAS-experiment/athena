/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Christian Grefe


#ifndef TAU_ANALYSIS_ALGORITHMS__TAU_TRUTH_DECORATIONS_ALG_H
#define TAU_ANALYSIS_ALGORITHMS__TAU_TRUTH_DECORATIONS_ALG_H

#include <AnaAlgorithm/AnaReentrantAlgorithm.h>
#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgDataHandles/WriteDecorHandleKey.h>
#include <AsgTools/PropertyWrapper.h>
#include <xAODTau/TauJetContainer.h>


namespace CP
{
  /// \brief an algorithm to decorate truth matched information

  class TauTruthDecorationsAlg final : public EL::AnaReentrantAlgorithm
  {
    /// \brief the standard constructor
  public:
    using EL::AnaReentrantAlgorithm::AnaReentrantAlgorithm;
    StatusCode initialize () override;
    StatusCode execute (const EventContext &ctx) const override;

    /// \brief the tau collection we run on
  private:
    SG::ReadHandleKey<xAOD::TauJetContainer> m_tausKey { this, "taus", "", "the input tau jet container" };

    /// \brief the decoration for the tau scale factor
  private:
    Gaudi::Property<std::vector<std::string>> m_doubleDecorations {this, "doubleDecorations", {}, "the list decorations with type double to copy"};
    Gaudi::Property<std::vector<std::string>> m_floatDecorations {this, "floatDecorations", {}, "the list decorations with type float to copy"};
    Gaudi::Property<std::vector<std::string>> m_intDecorations {this, "intDecorations", {}, "the list decorations with type int to copy"};
    Gaudi::Property<std::vector<std::string>> m_unsignedIntDecorations {this, "unsignedIntDecorations", {}, "the list decorations with type unsigned int to copy"};
    Gaudi::Property<std::vector<std::string>> m_charDecorations {this, "charDecorations", {}, "the list decorations with type char to copy"};
    Gaudi::Property<std::string> m_prefix {this, "prefix", "truth_", "the prefix to be added to all output decorations"};

    // the mapping of double to float is intentional to save disk space
    std::unordered_map<std::unique_ptr<SG::AuxElement::ConstAccessor<double>>, SG::WriteDecorHandleKey<xAOD::TauJetContainer>> m_doubleWriteHandleKeys;
    std::unordered_map<std::unique_ptr<SG::AuxElement::ConstAccessor<float>>, SG::WriteDecorHandleKey<xAOD::TauJetContainer>> m_floatWriteHandleKeys;
    std::unordered_map<std::unique_ptr<SG::AuxElement::ConstAccessor<int>>, SG::WriteDecorHandleKey<xAOD::TauJetContainer>> m_intWriteHandleKeys;
    std::unordered_map<std::unique_ptr<SG::AuxElement::ConstAccessor<unsigned int>>, SG::WriteDecorHandleKey<xAOD::TauJetContainer>> m_unsignedIntWriteHandleKeys;
    std::unordered_map<std::unique_ptr<SG::AuxElement::ConstAccessor<char>>, SG::WriteDecorHandleKey<xAOD::TauJetContainer>> m_charWriteHandleKeys;

    SG::WriteDecorHandleKey<xAOD::TauJetContainer> m_truthDecayModeKey {
      this, "decayModeDecoration", "truth_DecayMode", "the decoration for the tau decay mode"};
    SG::WriteDecorHandleKey<xAOD::TauJetContainer> m_truthParticleTypeKey {
      this, "particleTypeDecoration", "truth_ParticleType", "the decoration for the tau particle type"};

    SG::WriteDecorHandleKey<xAOD::TauJetContainer> m_partonTruthLabelIDKey {
      this, "partonTruthLabelIDDecoration", "truth_PartonTruthLabelID", "the decoration for the tau parton truth label ID from the linked jet"};
  };
}

#endif
