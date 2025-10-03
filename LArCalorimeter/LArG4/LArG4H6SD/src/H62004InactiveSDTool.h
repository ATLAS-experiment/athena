/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARG4H62004SD_H62004INACTIVESDTOOL_H
#define LARG4H62004SD_H62004INACTIVESDTOOL_H

// System includes
#include <string>
#include <vector>

// Local includes
#include "H62004CalibSDTool.h"
#include "LArG4Code/ILArCalibCalculatorSvc.h"

namespace LArG4
{

  /// @class H62004InactiveSDTool
  /// @brief Tool for constructing H62004 calib SDs for inactive material.
  ///
  /// Based on the previous LArG4H62004InactiveSDTool implementation.
  ///
  /// This implementation uses the LAr SD wrapper design for managing multiple
  /// SDs when running multi-threaded. See ATLASSIM-2606 for discussions.
  ///
  class H62004InactiveSDTool : public H62004CalibSDTool
  {

  public:

    /// Constructor
    H62004InactiveSDTool(const std::string& type, const std::string& name,
                         const IInterface* parent);

  private:

    StatusCode initializeCalculators() override final;

    /// Create the SD wrapper for current worker thread
    G4VSensitiveDetector* makeSD() const override final;

    /// Hit collection name
    std::string m_hitCollName{"LArCalibrationHitInactive"};

    ServiceHandle<ILArCalibCalculatorSvc> m_emepiwcalc {this, "EMECPosIWCalibrationCalculator", "EMECPosInnerWheelCalibrationCalculator"};
    ServiceHandle<ILArCalibCalculatorSvc> m_heccalc {this, "HECWheelInactiveCalculator", "LocalCalibrationInactiveCalculator"};
    ServiceHandle<ILArCalibCalculatorSvc> m_fcal1calc {this, "FCAL1CalibCalculator", "LArFCAL1H62004CalibCalculator"};
    ServiceHandle<ILArCalibCalculatorSvc> m_fcal2calc {this, "FCAL2CalibCalculator", "LArFCAL2H62004CalibCalculator"};

    /// @name SD volumes
    /// @{
    Gaudi::Property<std::vector<std::string>> m_emecVolumes {this, "EMECVolumes"};
    Gaudi::Property<std::vector<std::string>> m_hecVolumes {this, "HECVolumes"};
    Gaudi::Property<std::vector<std::string>> m_fcal1Volumes {this, "FCAL1Volumes"};
    Gaudi::Property<std::vector<std::string>> m_fcal2Volumes {this, "FCAL2Volumes"};
    /// @}
  }; // class H62004InactiveSDTool

} // namespace LArG4

#endif
