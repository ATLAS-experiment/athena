/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



#ifndef EGAMMA_ANALYSIS_ALGORITHMS__EGAMMA_CALIBRATION_AND_SMEARING_ALG_H
#define EGAMMA_ANALYSIS_ALGORITHMS__EGAMMA_CALIBRATION_AND_SMEARING_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgTools/PropertyWrapper.h>
#include <EgammaAnalysisInterfaces/IEgammaCalibrationAndSmearingTool.h>
#include <EgammaAnalysisAlgorithms/CopyHelpers.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SelectionHelpers/OutOfValidityHelper.h>
#include <SystematicsHandles/SysCopyHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>

namespace CP
{
  /// \brief an algorithm for calling \ref
  /// CP::IEgammaCalibrationAndSmearingTool

  class EgammaCalibrationAndSmearingAlg final : public EL::AnaAlgorithm
  {
    /// \brief the standard constructor
  public:
    using EL::AnaAlgorithm::AnaAlgorithm;
    StatusCode initialize () override;
    StatusCode execute () override;



    /// \brief the smearing tool
  private:
    ToolHandle<CP::IEgammaCalibrationAndSmearingTool> m_calibrationAndSmearingTool {this, "calibrationAndSmearingTool", "EgammaCalibrationAndSmearingTool", "the smearing tool  we apply"};

    /// \brief whether to ignore all tool systematics
    ///
    /// The tool can be configured only to run the non-systematics or
    /// only the systematics part of the corrections.  That means this
    /// algorithm needs to be scheduled twice, once without
    /// systematics and once with.  Besides configuring the tools for
    /// their respective roles the first algorithm should have this
    /// property set to avoid reporting any systematics or trying to
    /// request any systematics.
  private:
    Gaudi::Property<bool> m_noToolSystematics {
      this, "noToolSystematics", false, "whether to ignore all tool systematics"};

    /// \brief whether to skip the nominal correction (for PHYSLITE)
  private:
    Gaudi::Property<bool> m_skipNominal {
      this, "skipNominal", false, "whether to skip the nominal correction (for PHYSLITE)"};

    /// \brief the systematics list we run
  private:
    SysListHandle m_systematicsList {this};

    /// \brief the egamma collection we run on
  private:
    SysCopyHandle<xAOD::EgammaContainer> m_egammaHandle {
      this, "egammas", "Electrons", "the egamma collection to run on"};

    /// \brief the preselection we apply to our input
  private:
    SysReadSelectionHandle m_preselection {
      this, "preselection", "", "the preselection to apply"};

    /// \brief decoration for calorimeter cluster energy resolution
  private:
    SysWriteDecorHandle<float> m_caloClusterEnergyResoHandle {
      this, "caloClusterEnergyReso", "caloClusterEnergyReso_%SYS%", "decoration for calorimeter cluster energy resolution" };

    /// \brief the helper for OutOfValidity results
  private:
    OutOfValidityHelper m_outOfValidity {this};
  };
}

#endif
