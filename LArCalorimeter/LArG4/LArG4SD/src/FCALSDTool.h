/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARG4SD_FCALSDTOOL_H
#define LARG4SD_FCALSDTOOL_H

// System includes
#include <string>
#include <vector>

// Project includes
#include "LArG4Code/SimpleSDTool.h"
#include "LArG4Code/ILArCalculatorSvc.h"

namespace LArG4
{

  /// @class FCALSDTool
  /// @brief SD tool which manages the LAr forward calo sensitive detectors.
  ///
  /// NOTE: this design is in flux, migrating to be more multi-threading-friendly
  ///
  class FCALSDTool : public SimpleSDTool
  {
  public:

    /// Constructor
    FCALSDTool(const std::string& type, const std::string& name,
	       const IInterface* parent);

  private:
    /// Initialize Calculator Services
    StatusCode initializeCalculators() override final;
    
    /// Create the SD wrapper for current worker thread
    G4VSensitiveDetector* makeSD() const override final;

    /// List of volumes for each SD
    /// @{
    Gaudi::Property<std::vector<std::string>> m_fcal1Volumes{this, "FCAL1Volumes"};
    Gaudi::Property<std::vector<std::string>> m_fcal2Volumes{this, "FCAL2Volumes"};
    Gaudi::Property<std::vector<std::string>> m_fcal3Volumes{this, "FCAL3Volumes"};
    /// @}
    
    ServiceHandle<ILArCalculatorSvc> m_fcal1calc{this, "FCAL1Calculator"
      , "FCAL1Calculator"}; //LArFCAL1Calculator::GetInstance()
    ServiceHandle<ILArCalculatorSvc> m_fcal2calc{this, "FCAL2Calculator"
      , "FCAL2Calculator"}; //LArFCAL2Calculator::GetInstance()
    ServiceHandle<ILArCalculatorSvc> m_fcal3calc{this, "FCAL3Calculator"
      , "FCAL3Calculator"}; //LArFCAL3Calculator::GetInstance()
  }; // class FCALSDTool
} // namespace LArG4
#endif
