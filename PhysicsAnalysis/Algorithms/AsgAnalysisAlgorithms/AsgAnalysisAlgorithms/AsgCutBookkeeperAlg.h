/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Tadej Novak


#ifndef ASG_ANALYSIS_ALGORITHMS__ASG_CUT_BOOKKEEPER_ALG_H
#define ASG_ANALYSIS_ALGORITHMS__ASG_CUT_BOOKKEEPER_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgServices/ServiceHandle.h>
#include <PATInterfaces/SystematicSet.h>
#include <PMGAnalysisInterfaces/IPMGTruthWeightTool.h>
#include <SystematicsHandles/ISystematicsSvc.h>
#include <xAODCutFlow/CutBookkeeper.h>
#include <AsgTools/PropertyWrapper.h>

namespace CP
{
  /// \brief an algorithm for dumping the CutBookkeeper metadata
  /// into a histogram

  class AsgCutBookkeeperAlg final : public EL::AnaAlgorithm
  {
    /// \brief the standard constructor
  public:
    using EL::AnaAlgorithm::AnaAlgorithm;

  public:
    /// \brief initialize
    StatusCode initialize () override;

  public:
    /// \brief run once on each file
    StatusCode fileExecute () override;

  public:
    /// \brief finalize
    StatusCode finalize () override;

  private:
    /// \brief process a CutBookkeeper
    void processCutBookkeeper (const xAOD::CutBookkeeper *cbk,
                               size_t index);

    /// \brief flag to enable systematics
  private:
    Gaudi::Property<bool> m_enableSystematics {this, "enableSystematics", false, "enable systematics"};

    /// \brief the truth weight tool
  private:
    ToolHandle<PMGTools::IPMGTruthWeightTool> m_truthWeightTool {this, "truthWeightTool", "PMGTools::PMGTruthWeightTool", "the truth weight tool"};

    /// \brief the systematics service
  private:
    ServiceHandle<ISystematicsSvc> m_systematics {this, "systematics", "SystematicsSvc", "systematics service"};

    /// \brief run number we are processing
  private:
    Gaudi::Property<uint32_t> m_runNumber {this, "runNumber", 0, "the run number we are processing"};

    /// \brief MC channel number we are processing
  private:
    uint32_t m_mcChannelNumber {};

    /// \brief weights helper struct
  private:
    struct WeightsGroup
    {
      float nEventsProcessed {};
      float sumOfWeights {};
      float sumOfWeightsSquared {};
    };

    /// \brief weights map
  private:
    std::unordered_map<size_t, WeightsGroup> m_weights;

    /// \brief the pattern for histogram names
  private:
    Gaudi::Property<std::string> m_histPattern {this, "histPattern", "CutBookkeeper_%DSID%_%RUN%_%SYS%", "the pattern for histogram names"};
  };
}

#endif
