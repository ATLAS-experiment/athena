/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARG4SD_INACTIVESDTOOL_H
#define LARG4SD_INACTIVESDTOOL_H

// System includes
#include <string>
#include <vector>

// Project includes
#include "LArG4Code/CalibSDTool.h"
#include "LArG4Code/ILArCalibCalculatorSvc.h"

namespace LArG4
{

  /// @class InactiveSDTool
  /// @brief Sensitive detector tool which manages inactive-area LAr calib SDs.
  ///
  /// Design is in flux.
  ///
  class InactiveSDTool : public CalibSDTool
  {  
  public:
    /// Constructor
    InactiveSDTool(const std::string& type, const std::string& name,
		   const IInterface* parent);
    
  private:

    /// Initialize Calculator Services
    StatusCode initializeCalculators() override final;

    /// Create the SD wrapper for current worker thread
    G4VSensitiveDetector* makeSD() const override final;

    Gaudi::Property<std::vector<std::string>> m_barPreVolumes{this, "BarrelPreVolumes"};
    Gaudi::Property<std::vector<std::string>> m_barVolumes{this, "BarrelVolumes"};
    Gaudi::Property<std::vector<std::string>> m_ECPosInVolumes{this, "ECPosInVolumes"};
    Gaudi::Property<std::vector<std::string>> m_ECPosOutVolumes{this, "ECPosOutVolumes"};
    Gaudi::Property<std::vector<std::string>> m_ECNegInVolumes{this, "ECNegInVolumes"};
    Gaudi::Property<std::vector<std::string>> m_ECNegOutVolumes{this, "ECNegOutVolumes"};
    Gaudi::Property<std::vector<std::string>> m_HECWheelVolumes{this, "HECWheelVolumes"};
    Gaudi::Property<std::vector<std::string>> m_fcal1Volumes{this, "FCAL1Volumes"};
    Gaudi::Property<std::vector<std::string>> m_fcal2Volumes{this, "FCAL2Volumes"};
    Gaudi::Property<std::vector<std::string>> m_fcal3Volumes{this, "FCAL3Volumes"};

    /// Hit collection name
    Gaudi::Property<std::string> m_hitCollName{this, "HitCollectionName", "LArCalibrationHitInactive"};

    ServiceHandle<ILArCalibCalculatorSvc> m_embpscalc{this, "EMBPSCalibrationCalculator"
      , "BarrelPresamplerCalibrationCalculator"}; //BarrelPresampler::CalibrationCalculator
    ServiceHandle<ILArCalibCalculatorSvc> m_embcalc{this, "EMBCalibrationCalculator"
      , "BarrelCalibrationCalculator"}; //Barrel::CalibrationCalculator
    ServiceHandle<ILArCalibCalculatorSvc> m_emepiwcalc{this, "EMECPosIWCalibrationCalculator"
      , "EMECPosInnerWheelCalibrationCalculator"};//LArG4::EC::CalibrationCalculator(LArWheelCalculator::InnerAbsorberWheel, 1)
    ServiceHandle<ILArCalibCalculatorSvc> m_emeniwcalc{this, "EMECNegIWCalibrationCalculator"
      , "EMECNegInnerWheelCalibrationCalculator"};//LArG4::EC::CalibrationCalculator(LArWheelCalculator::InnerAbsorberWheel, -1)
    ServiceHandle<ILArCalibCalculatorSvc> m_emepowcalc{this, "EMECPosOWCalibrationCalculator"
      , "EMECPosOuterWheelCalibrationCalculator"}; //LArG4::EC::CalibrationCalculator(LArWheelCalculator::OuterAbsorberWheel, 1)
    ServiceHandle<ILArCalibCalculatorSvc> m_emenowcalc{this, "EMECNegOWCalibrationCalculator"
      , "EMECNegOuterWheelCalibrationCalculator"}; //LArG4::EC::CalibrationCalculator(LArWheelCalculator::OuterAbsorberWheel, -1)
    ServiceHandle<ILArCalibCalculatorSvc> m_heccalc{this, "HECWheelInactiveCalculator"
      , "HECCalibrationWheelInactiveCalculator"};  //LArG4::HEC::LArHECCalibrationWheelCalculator(LArG4::HEC::kWheelInactive)
    ServiceHandle<ILArCalibCalculatorSvc> m_fcal1calc{this, "FCAL1CalibCalculator"
      , "FCAL1CalibCalculator"};
    ServiceHandle<ILArCalibCalculatorSvc> m_fcal2calc{this, "FCAL2CalibCalculator"
      , "FCAL2CalibCalculator"};
    ServiceHandle<ILArCalibCalculatorSvc> m_fcal3calc{this, "FCAL3CalibCalculator"
      , "FCAL3CalibCalculator"};
  }; // class InactiveSDTool
} // namespace LArG4
#endif
