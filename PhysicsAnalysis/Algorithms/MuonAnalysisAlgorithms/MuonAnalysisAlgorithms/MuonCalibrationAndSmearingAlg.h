/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



#ifndef MUON_ANALYSIS_ALGORITHMS__MUON_CALIBRATION_AND_SMEARING_ALG_H
#define MUON_ANALYSIS_ALGORITHMS__MUON_CALIBRATION_AND_SMEARING_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgTools/PropertyWrapper.h>
#include <MuonAnalysisInterfaces/IMuonCalibrationAndSmearingTool.h>
#include <SelectionHelpers/OutOfValidityHelper.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SystematicsHandles/SysCopyHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <xAODMuon/MuonContainer.h>

namespace CP
{
  /// \brief an algorithm for calling \ref IMuonCalibrationAndSmearingTool

  class MuonCalibrationAndSmearingAlg final : public EL::AnaAlgorithm
  {
    /// \brief the standard constructor
  public:
    using EL::AnaAlgorithm::AnaAlgorithm;
    StatusCode initialize () override;
    StatusCode execute () override;
    


    /// \brief the smearing tool
  private:
    ToolHandle<IMuonCalibrationAndSmearingTool> m_calibrationAndSmearingTool {this, "calibrationAndSmearingTool", "CP::MuonCalibrationAndSmearingTool", "the calibration and smearing tool we apply"};

    /// \brief whether to skip the nominal correction (for PHYSLITE)
  private:
    Gaudi::Property<bool> m_skipNominal {
      this, "skipNominal", false, "whether to skip the nominal correction (for PHYSLITE)"};

    /// \brief the systematics list we run
  private:
    SysListHandle m_systematicsList {this};

    /// \brief the muon collection we run on
  private:
    SysCopyHandle<xAOD::MuonContainer> m_muonHandle {
      this, "muons", "Muons", "the muon collection to run on"};

    /// \brief the preselection we apply to our input
  private:
    SysReadSelectionHandle m_preselection {
      this, "preselection", "", "the preselection to apply"};

    /// \brief the helper for OutOfValidity results
  private:
    OutOfValidityHelper m_outOfValidity {this};
  };
}

#endif
