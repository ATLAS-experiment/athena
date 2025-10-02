/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARG4H62004SD_H62004FCALSDTOOL_H
#define LARG4H62004SD_H62004FCALSDTOOL_H

// System includes
#include <string>
#include <vector>

// Local includes
#include "H62004SimpleSDTool.h"
#include "LArG4Code/ILArCalculatorSvc.h"

namespace LArG4
{

  /// @class H62004FCALSDTool
  /// @brief Tool for constructing H62004 SDs for FCAL.
  ///
  /// Based on the previous LArG4H62004FCALSDTool implementation.
  ///
  /// This implementation uses the LAr SD wrapper design for managing multiple
  /// SDs when running multi-threaded. See ATLASSIM-2606 for discussions.
  ///
  class H62004FCALSDTool : public H62004SimpleSDTool
  {

  public:

    /// Constructor
    H62004FCALSDTool(const std::string& type, const std::string& name,
                     const IInterface* parent);

  private:

    StatusCode initializeCalculators() override final;

    /// Create the SD wrapper for current worker thread
    G4VSensitiveDetector* makeSD() const override final;

    /// Hit collection name
    std::string m_hitCollName {"LArHitFCAL"};

    ServiceHandle<ILArCalculatorSvc> m_fcal1calc {this, "FCAL1Calculator", "FCAL1Calculator"};
    ServiceHandle<ILArCalculatorSvc> m_fcal2calc {this, "FCAL2Calculator", "FCAL2Calculator"};
    ServiceHandle<ILArCalculatorSvc> m_fcalcoldcalc {this, "FCALColdCalculator", "FCALColdCalculator"};
    /// @name SD volumes
    /// @{
    Gaudi::Property<std::vector<std::string>> m_fcal1Volumes {this, "FCAL1Volumes"};
    Gaudi::Property<std::vector<std::string>> m_fcal2Volumes {this, "FCAL2Volumes"};
    Gaudi::Property<std::vector<std::string>> m_fcalColdVolumes {this, "FCALColdVolumes"};
    /// @}

  }; // class H62004FCALSDTool

} // namespace LArG4

#endif
