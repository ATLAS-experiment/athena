/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef ASG_ANALYSIS_ALGORITHMS__OVERLAP_REMOVAL_ALG_H
#define ASG_ANALYSIS_ALGORITHMS__OVERLAP_REMOVAL_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AssociationUtils/ToolBox.h>
#include <SelectionHelpers/SysWriteSelectionHandle.h>
#include <SystematicsHandles/SysCopyHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <AsgTools/PropertyWrapper.h>

namespace CP
{
  /// \brief an algorithm for calling \ref IPileupReweightingTool

  class OverlapRemovalAlg final : public EL::AnaAlgorithm
  {
    /// \brief the standard constructor
  public:
    using EL::AnaAlgorithm::AnaAlgorithm;
    StatusCode initialize () override;
    StatusCode execute () override;



    /// The OR toolbox
  private:
    ToolHandle<ORUtils::IOverlapRemovalTool> m_overlapTool { this, "overlapTool", "ORUtils::OverlapRemovalTool", "the overlap removal tool"};

    /// \brief the systematics list we run
  private:
    SysListHandle m_systematicsList {this};

  private:
    SysCopyHandle<const xAOD::ElectronContainer> m_electronsHandle {
      this, "electrons", "", "the electrons container to use"};
    SysCopyHandle<const xAOD::MuonContainer> m_muonsHandle {
      this, "muons", "", "the muons container to use"};
    SysCopyHandle<const xAOD::JetContainer> m_jetsHandle {
      this, "jets", "", "the jets container to use"};
    SysCopyHandle<const xAOD::TauJetContainer> m_tausHandle {
      this, "taus", "", "the taus container to use"};
    SysCopyHandle<const xAOD::PhotonContainer> m_photonsHandle {
      this, "photons", "", "the photons container to use"};
    SysCopyHandle<const xAOD::JetContainer> m_fatJetsHandle {
      this, "fatJets", "", "the fat jets container to use"};

    /// \brief the decoration for the overlap removal status
  private:
    SysWriteSelectionHandle m_electronsSelectionHandle {
      this, "electronsDecoration", "", "the decoration for the electron overlap removal selection"};
    SysWriteSelectionHandle m_muonsSelectionHandle {
      this, "muonsDecoration", "", "the decoration for the muon overlap removal selection"};
    SysWriteSelectionHandle m_jetsSelectionHandle {
      this, "jetsDecoration", "", "the decoration for the jet overlap removal selection"};
    SysWriteSelectionHandle m_tausSelectionHandle {
      this, "tausDecoration", "", "the decoration for the tau overlap removal selection"};
    SysWriteSelectionHandle m_photonsSelectionHandle {
      this, "photonsDecoration", "", "the decoration for the photon overlap removal selection"};
    SysWriteSelectionHandle m_fatJetsSelectionHandle {
      this, "fatJetsDecoration", "", "the decoration for the fat jet overlap removal selection"};

    /// \brief the tool output decoration for the overlap removal status
  private:
    Gaudi::Property<std::string> m_overlapRemovalDecoration {this, "OutputLabel", "", "the decoration for the overlap removal tool output"};

    /// \brief the accessor for \ref m_overlapRemovalDecoration
  private:
    std::unique_ptr<const SG::Accessor<char> > m_overlapRemovalAccessor;

  };
}

#endif
