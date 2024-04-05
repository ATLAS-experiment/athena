/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Christian Grefe


#ifndef TAU_ANALYSIS_ALGORITHMS__TAU_TRUTH_DECORATIONS_ALG_H
#define TAU_ANALYSIS_ALGORITHMS__TAU_TRUTH_DECORATIONS_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <TauAnalysisTools/ITauEfficiencyCorrectionsTool.h>
#include <SelectionHelpers/OutOfValidityHelper.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <xAODTau/TauJetContainer.h>

namespace CP
{
  /// \brief an algorithm to decorate truth matched information

  class TauTruthDecorationsAlg final : public EL::AnaAlgorithm
  {
    /// \brief the standard constructor
  public:
    using EL::AnaAlgorithm::AnaAlgorithm;
    StatusCode initialize () override;
    StatusCode execute () override;

    /// \brief the systematics list we run
  private:
    SysListHandle m_systematicsList {this};

    /// \brief the tau collection we run on
  private:
    SysReadHandle<xAOD::TauJetContainer> m_tauHandle {
      this, "taus", "TauJets", "the tau collection to run on"};

    /// \brief the preselection we apply to our input
  private:
    SysReadSelectionHandle m_preselection {
      this, "preselection", "", "the preselection to apply"};

    /// \brief the decoration for the tau scale factor
  private:
    Gaudi::Property<std::vector<std::string>> m_doubleDecorations {this, "doubleDecorations", {}, "the list decorations with type double to copy"};
    Gaudi::Property<std::vector<std::string>> m_floatDecorations {this, "floatDecorations", {}, "the list decorations with type float to copy"};
    Gaudi::Property<std::vector<std::string>> m_intDecorations {this, "intDecorations", {}, "the list decorations with type int to copy"};
    Gaudi::Property<std::vector<std::string>> m_charDecorations {this, "charDecorations", {}, "the list decorations with type char to copy"};
    Gaudi::Property<std::string> m_prefix {this, "prefix", "truth_", "the prefix to be added to all output decorations"};

    // the mapping of double to float is intentional to save disk space
    std::vector<std::pair<std::unique_ptr<SG::AuxElement::ConstAccessor<double>>, std::unique_ptr<SysWriteDecorHandle<float>>>> m_doubleWriteHandles;
    std::vector<std::pair<std::unique_ptr<SG::AuxElement::ConstAccessor<float>>, std::unique_ptr<SysWriteDecorHandle<float>>>> m_floatWriteHandles;
    std::vector<std::pair<std::unique_ptr<SG::AuxElement::ConstAccessor<int>>, std::unique_ptr<SysWriteDecorHandle<int>>>> m_intWriteHandles;
    std::vector<std::pair<std::unique_ptr<SG::AuxElement::ConstAccessor<char>>, std::unique_ptr<SysWriteDecorHandle<char>>>> m_charWriteHandles;

    SysWriteDecorHandle<int> m_truthDecayModeDecoration {
      this, "decayModeDecoration", "truth_DecayMode", "the decoration for the tau decay mode"};
  };
}

#endif
