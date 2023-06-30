/*
    Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef VKalVrt_EMERGINGJETSELECTORALG_H
#define VKalVrt_EMERGINGJETSELECTORALG_H

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>

namespace Rec
{

  /// \brief An algorithm for counting containers
  class EmergingJetSelectorAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
  public:
    EmergingJetSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

  private:
    // ToolHandle<whatever> handle {this, "pythonName", "defaultValue",
    // "someInfo"};

    SG::ReadHandleKey<xAOD::JetContainer> m_containerInKey{
      this, "jetContainerInKey", "", "containerName to read"};
    SG::WriteHandleKey<ConstDataVector<xAOD::JetContainer>> m_containerOutKey{
      this, "jetContainerOutKey", "", "containerName to write"};
    SG::ReadHandleKey<xAOD::EventInfo> m_EventInfoKey{
      this, "eventInfoKey", "EventInfo", "EventInfo container to dump"};

    /// @brief minimum jet pT
    Gaudi::Property<float> m_minPt{this, "minPt", 200000, "Minumum jet pT"};
    /// @brief minimum jet PTF
    Gaudi::Property<float> m_minPTF{this, "minPTF", 0,
	"Minumum jet prompt track fraction"};
    /// @brief minimum jet PTF
    Gaudi::Property<float> m_maxEta{this, "maxEta", 2.5, "Maximum jet eta"};

    /// @brief Flag to apply pT sorting to selected container
    Gaudi::Property<bool> m_pTsort{this, "pTsort", true,
	"Turn on jet pT ordering"};
    /// @brief Maximum number of jets to consider in the output
    Gaudi::Property<int> m_truncateAtAmount{this, "truncateAtAmount", -1,
	"Maximum number of jets to store"};

    std::unordered_map<std::string,
      SG::AuxElement::Decorator<std::vector<float>>>
      m_fourVecDecos;
  };
}

#endif
