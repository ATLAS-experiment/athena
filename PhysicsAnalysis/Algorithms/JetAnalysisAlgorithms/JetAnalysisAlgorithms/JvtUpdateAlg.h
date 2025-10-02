/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef JET_ANALYSIS_ALGORITHMS__JVT_UPDATE_ALG_H
#define JET_ANALYSIS_ALGORITHMS__JVT_UPDATE_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <JetInterface/IJetUpdateJvt.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SystematicsHandles/SysCopyHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <xAODJet/JetContainer.h>
#include <AsgTools/PropertyWrapper.h>

namespace CP
{
  /// \brief an algorithm for calling \ref IJetUpdateJvt

  class JvtUpdateAlg final : public EL::AnaAlgorithm
  {
    /// \brief the standard constructor
  public:
    using EL::AnaAlgorithm::AnaAlgorithm;
    StatusCode initialize () override;
    StatusCode execute () override;



    /// \brief the update tool
  private:
    ToolHandle<IJetUpdateJvt> m_jvtTool {this, "jvtTool", "", "the jvt tool we apply"};

    /// \brief the systematics list we run
  private:
    SysListHandle m_systematicsList {this};

    /// \brief the jet collection we run on
  private:
    SysCopyHandle<xAOD::JetContainer> m_jetHandle {
      this, "jets", "", "the jet collection to run on"};

    /// \brief the preselection we apply to our input
  private:
    SysReadSelectionHandle m_preselection {
      this, "preselection", "", "the preselection to apply"};

    /// \brief the name of the decoration we create
  private:
    Gaudi::Property<std::string> m_decorationName {this, "decorationName", "Jvt", "the decoration name to use"};

    /// \brief the decoration accessor we use
  private:
    std::unique_ptr<SG::Accessor<float> > m_decorationAccessor;
  };
}

#endif
