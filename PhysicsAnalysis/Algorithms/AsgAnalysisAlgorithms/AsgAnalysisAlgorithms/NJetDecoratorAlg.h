/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/



#ifndef ASG_ANALYSIS_ALGORITHMS__NJET_DECORATOR_ALG_H
#define ASG_ANALYSIS_ALGORITHMS__NJET_DECORATOR_ALG_H

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <AnaAlgorithm/AnaAlgorithm.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <AsgTools/PropertyWrapper.h>
#include <map>
#include <functional>

namespace CP
{
  /// \brief an algorithm for decorating EventInfo

  class NJetDecoratorAlg final : public EL::AnaAlgorithm
  {
  public:

    /// \brief the standard constructor
    using EL::AnaAlgorithm::AnaAlgorithm;
    StatusCode initialize () override;
    StatusCode execute () override;



  private:

    /// \brief the systematics list we run
    SysListHandle m_systematicsList {this};

    /// \brief the name of the event info object
    CP::SysReadHandle<xAOD::EventInfo> m_eventInfoHandle {this, "eventInfo", "EventInfo", "the input EventInfo object"};

    //// Input jets and selection
    CP::SysReadHandle<xAOD::JetContainer> m_jetsHandle{
	this, "jets", "", "the jet container to use"};
    CP::SysReadSelectionHandle m_jetSelection{
	this, "jetSelection", "", "the selection on the input jets"};
    CP::SysWriteDecorHandle<int> m_Njet_decor {
        this, "Njet", "Njet_%SYS%", "Number of jets"
    };
  };
}

#endif
