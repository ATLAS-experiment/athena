/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Tadej Novak



#ifndef F_TAG_ANALYSIS_ALGORITHMS__B_TAGGING_INFORMATION_DECORATION_ALG_H
#define F_TAG_ANALYSIS_ALGORITHMS__B_TAGGING_INFORMATION_DECORATION_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <FTagAnalysisInterfaces/IBTaggingSelectionTool.h>
#include <SelectionHelpers/OutOfValidityHelper.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <xAODJet/JetContainer.h>
#include <AsgTools/PropertyWrapper.h>

namespace CP
{
  class BTaggingInformationDecoratorAlg final : public EL::AnaAlgorithm
  {
    /// \brief the standard constructor
  public:
    using EL::AnaAlgorithm::AnaAlgorithm;
    StatusCode initialize () override;
    StatusCode execute () override;
    

    /// \brief the systematics list we run
  private:
    SysListHandle m_systematicsList {this};

    /// \brief the selection tool
  private:
    ToolHandle<IBTaggingSelectionTool> m_selectionTool {this, "selectionTool", "", "the b-tagging selection tool"};

    /// \brief the jets continer we run on
  private:
    SysReadHandle<xAOD::JetContainer> m_jetHandle {
      this, "jets", "", "the jets collection to run on"};

    /// \brief the preselection we apply to our input
  private:
    SysReadSelectionHandle m_preselection {
      this, "preselection", "", "the preselection to apply"};

    /// \brief the helper for OutOfValidity results
  private:
    OutOfValidityHelper m_outOfValidity {this};

    /// \brief the decoration for the b-tagging weight
  private:
    Gaudi::Property<std::string> m_taggerWeightDecoration {this, "taggerWeightDecoration", "", "the decoration for the tagger weight"};

    /// \brief the decorator for \ref m_taggerWeightDecoration
  private:
    std::unique_ptr<const SG::AuxElement::Decorator<float> > m_taggerWeightDecorator {};

    /// \brief the decoration for the b-tagging quantiles
  private:
    Gaudi::Property<std::string> m_quantileDecoration {this, "quantileDecoration", "", "the decoration for the continuous WP quantile"};

    /// \brief the decorator for \ref m_quantileDecoration
  private:
    std::unique_ptr<const SG::AuxElement::Decorator<int> > m_quantileDecorator {};
  };
}

#endif
