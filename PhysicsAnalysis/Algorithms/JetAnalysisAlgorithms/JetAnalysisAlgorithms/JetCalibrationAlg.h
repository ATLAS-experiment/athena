/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef JET_ANALYSIS_ALGORITHMS__JET_CALIBRATION_ALG_H
#define JET_ANALYSIS_ALGORITHMS__JET_CALIBRATION_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <JetCalibTools/IJetCalibrationTool.h>
#include <SystematicsHandles/SysCopyHandle.h>
#include <SystematicsHandles/SysListHandle.h>

namespace CP
{
  /// \brief an algorithm for calling \ref IJetCalibrationTool

  class JetCalibrationAlg final : public EL::AnaAlgorithm
  {
    /// \brief the standard constructor
  public:
    using EL::AnaAlgorithm::AnaAlgorithm;
    StatusCode initialize () override;
    StatusCode execute () override;
    


    /// \brief the calibration tool
  private:
    ToolHandle<IJetCalibrationTool> m_calibrationTool {this, "calibrationTool", "JetCalibrationTool", "the calibration tool we apply"};

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
