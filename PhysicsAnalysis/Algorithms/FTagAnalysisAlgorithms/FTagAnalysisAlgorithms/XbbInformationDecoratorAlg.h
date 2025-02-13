/*
  Copyright (C) 2002 - 2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Binbin Dong

#ifndef F_TAG_ANALYSIS_ALGORITHMS_XBB_INFORMATION_DECORATOR_H
#define F_TAG_ANALYSIS_ALGORITHMS_XBB_INFORMATION_DECORATOR_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <FTagAnalysisInterfaces/IBTaggingSelectionJsonTool.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <xAODJet/JetContainer.h>
#include <AsgTools/PropertyWrapper.h>

namespace CP
{
  class XbbInformationDecoratorAlg : public EL::AnaAlgorithm
  {
  /// \brief the standard constructor
  public:
    using EL::AnaAlgorithm::AnaAlgorithm;
    virtual StatusCode initialize() override;
    virtual StatusCode execute() override;

  /// \brief the systematics list we run
  private:
    SysListHandle m_systematicsList {this};

  /// \brief the selection tool
  private:
    ToolHandle<IBTaggingSelectionJsonTool> m_selectionTool {this, "selectionTool", "", "xbb selection json tool"};

  /// \brief the jet container we run on
  private:
    SysReadHandle<xAOD::JetContainer> m_jetHandle {this, "jets", "", "the jet collection to run on"};

  /// \brief the preselection we apply to our input
  private:
    SysReadSelectionHandle m_preselection {
      this, "preselection", "", "the preselection to apply"};

  /// \brief the decoration for the xbb decision
  private:
    Gaudi::Property<std::string> m_taggerDecisionDecoration {this, "taggerDecisionDecoration", "", "the decoration for the taggerdecision"};

  /// \brief the decorator for \ref m_taggerDecisionDecoration
  private:
    std::unique_ptr<const SG::AuxElement::Decorator<int> > m_taggerDecisionDecorator {};
  };
}
#endif 
