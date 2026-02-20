/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author P-A Delsart
///
/// This jet alg is meant to replace the JetCalibrationAlg.
/// The version here simply duplicates JetCalibrationAlg but using the new JetCalibTool instead of JetCalibrationTool.
/// The 2 versions will co-exist while transitionning from the old calib tool to the new after which the old versions will be removed


#ifndef JET_ANALYSIS_ALGORITHMS__JET_CALIB_ALG_H
#define JET_ANALYSIS_ALGORITHMS__JET_CALIB_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <SystematicsHandles/SysCopyHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <JetAnalysisInterfaces/IJetCalibTool.h>


namespace CP
{
  /// \brief an algorithm for calling \ref IJetCalibrationTool

  class JetCalibAlg final : public EL::AnaAlgorithm
  {
    /// \brief the standard constructor
  public:
    using EL::AnaAlgorithm::AnaAlgorithm;
    StatusCode initialize () override;
    StatusCode execute () override;



    /// \brief the calibration tool
  private:
    ToolHandle<IJetCalibTool> m_calibrationTool {this, "calibrationTool", "JetCalibrationTool", "the calibration tool we apply"};

    /// \brief the systematics list we run
  private:
    SysListHandle m_systematicsList {this};

    /// \brief the jet collection we run on
  private:
    SysCopyHandle<xAOD::JetContainer> m_jetHandle {
      this, "jets", "", "the jet collection to run on"};
  };
}

#endif
