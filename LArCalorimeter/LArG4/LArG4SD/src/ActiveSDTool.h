/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARG4SD_ACTIVESDTOOL_H
#define LARG4SD_ACTIVESDTOOL_H

// System includes
#include <string>
#include <vector>

// Project includes
#include "LArG4Code/CalibSDTool.h"
#include "LArG4Code/ILArCalibCalculatorSvc.h"

namespace LArG4
{

  /// @class ActiveSDTool
  /// @brief Sensitive detector tool which manages activate-area LAr calib SDs.
  ///
  /// Design is in flux.
  ///
  class ActiveSDTool : public CalibSDTool
  {
  public:
    /// Constructor
    ActiveSDTool(const std::string& type, const std::string& name,
		 const IInterface* parent);

  private:
    /// Initialize Calculator Services
    StatusCode initializeCalculators() override final;

    /// Create the SD wrapper for current worker thread
    G4VSensitiveDetector* makeSD() const override final;

    /// Hit collection name
    Gaudi::Property<std::string> m_hitCollName{this, "HitCollectionName", "LArCalibrationHitActive"};

    /// @name SD volumes
    /// @{
    Gaudi::Property<std::vector<std::string>> m_stacVolumes{this, "StacVolumes"};
    Gaudi::Property<std::vector<std::string>> m_presBarVolumes{this, "PresamplerVolumes"};
    Gaudi::Property<std::vector<std::string>> m_posIWVolumes{this, "PosIWVolumes"};
    Gaudi::Property<std::vector<std::string>> m_negIWVolumes{this, "NegIWVolumes"};
    Gaudi::Property<std::vector<std::string>> m_posOWVolumes{this, "PosOWVolumes"};
    Gaudi::Property<std::vector<std::string>> m_negOWVolumes{this, "NegOWVolumes"};
    Gaudi::Property<std::vector<std::string>> m_presECVolumes{this, "PresVolumes"};
    Gaudi::Property<std::vector<std::string>> m_pBOBVolumes{this, "PosBOBarretteVolumes"};
    Gaudi::Property<std::vector<std::string>> m_nBOBVolumes{this, "NegBOBarretteVolumes"};
    Gaudi::Property<std::vector<std::string>> m_fcal1Volumes{this, "FCAL1Volumes"};
    Gaudi::Property<std::vector<std::string>> m_fcal2Volumes{this, "FCAL2Volumes"};
    Gaudi::Property<std::vector<std::string>> m_fcal3Volumes{this, "FCAL3Volumes"};
    Gaudi::Property<std::vector<std::string>> m_sliceVolumes{this, "SliceVolumes"};
    /// @}

    ServiceHandle<ILArCalibCalculatorSvc> m_bpsmodcalc{this, "EMBPSCalibrationCalculator"
      , "BarrelPresamplerCalibrationCalculator"}; //LArG4::BarrelPresampler::CalibrationCalculator
    ServiceHandle<ILArCalibCalculatorSvc> m_embcalc{this, "EMBCalibrationCalculator"
      , "BarrelCalibrationCalculator"};    //LArG4::Barrel::CalibrationCalculator
    ServiceHandle<ILArCalibCalculatorSvc> m_emepiwcalc{this, "EMECPosIWCalibrationCalculator"
      , "EMECPosInnerWheelCalibrationCalculator"}; //LArG4::EC::CalibrationCalculator(LArWheelCalculator::InnerAbsorberWheel, 1)
    ServiceHandle<ILArCalibCalculatorSvc> m_emeniwcalc{this, "EMECNegIWCalibrationCalculator"
      , "EMECNegInnerWheelCalibrationCalculator"}; //LArG4::EC::CalibrationCalculator(LArWheelCalculator::InnerAbsorberWheel, -1)
    ServiceHandle<ILArCalibCalculatorSvc> m_emepowcalc{this, "EMECPosOWCalibrationCalculator"
      , "EMECPosOuterWheelCalibrationCalculator"}; //LArG4::EC::CalibrationCalculator(LArWheelCalculator::OuterAbsorberWheel, 1)
    ServiceHandle<ILArCalibCalculatorSvc> m_emenowcalc{this, "EMECNegOWCalibrationCalculator"
      , "EMECNegOuterWheelCalibrationCalculator"}; //LArG4::EC::CalibrationCalculator(LArWheelCalculator::OuterAbsorberWheel, -1)
    ServiceHandle<ILArCalibCalculatorSvc> m_emepscalc{this, "EMECPSCalibrationCalculator"
      , "EMECPresamplerCalibrationCalculator"}; //LArG4::EC::PresamplerCalibrationCalculator
    ServiceHandle<ILArCalibCalculatorSvc> m_emepobarcalc{this, "EMECPosBOBCalibrationCalculator"
      , "EMECPosBackOuterBarretteCalibrationCalculator"}; //LArG4::EC::CalibrationCalculator(LArWheelCalculator::BackOuterBarretteWheelCalib, 1)
    ServiceHandle<ILArCalibCalculatorSvc> m_emenobarcalc{this, "EMECNegBOBCalibrationCalculator"
      , "EMECNegBackOuterBarretteCalibrationCalculator"}; //LArG4::EC::CalibrationCalculator(LArWheelCalculator::BackOuterBarretteWheelCalib, 1)
    ServiceHandle<ILArCalibCalculatorSvc> m_heccalc{this, "HECWActiveCalculator"
      , "HECCalibrationWheelActiveCalculator"};   //LArG4::HEC::LArHECCalibrationWheelCalculator(LArG4::HEC::kWheelActive)
    ServiceHandle<ILArCalibCalculatorSvc> m_fcal1calc{this, "FCAL1CalibCalculator", "FCAL1CalibCalculator"};
    ServiceHandle<ILArCalibCalculatorSvc> m_fcal2calc{this, "FCAL2CalibCalculator", "FCAL2CalibCalculator"};
    ServiceHandle<ILArCalibCalculatorSvc> m_fcal3calc{this, "FCAL3CalibCalculator", "FCAL3CalibCalculator"};
  }; // class ActiveSDTool
} // namespace LArG4
#endif
