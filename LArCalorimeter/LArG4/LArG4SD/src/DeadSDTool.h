/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARG4SD_DEADSDTOOL_H
#define LARG4SD_DEADSDTOOL_H

// System includes
#include <string>
#include <vector>

// Project includes
#include "LArG4Code/CalibSDTool.h"

namespace LArG4
{

  class DeadSDTool : public CalibSDTool
  {
  public:
    // Constructor
    DeadSDTool(const std::string& type, const std::string& name,
	       const IInterface* parent);

  private:
    /// Initialize Calculator Services
    StatusCode initializeCalculators() override final;

    /// Create the SD wrapper for current worker thread
    G4VSensitiveDetector* makeSD() const override final;

    /// Hit collection name
    Gaudi::Property<std::string> m_hitCollName{this, "HitCollectionName", "LArCalibrationHitDeadMaterial"};

    /// Do we add the escaped energy processing?
    /// This is only in "mode 1" (Tile+LAr), not in "DeadLAr" mode
    Gaudi::Property<bool> m_do_eep{this, "doEscapedEnergy", false};

    Gaudi::Property<std::vector<std::string>> m_barCryVolumes{this, "BarrelCryVolumes"};
    Gaudi::Property<std::vector<std::string>> m_barCryLArVolumes{this, "BarrelCryLArVolumes"};
    Gaudi::Property<std::vector<std::string>> m_barCryMixVolumes{this, "BarrelCryMixVolumes"};
    Gaudi::Property<std::vector<std::string>> m_DMVolumes{this, "DeadMaterialVolumes"};
    Gaudi::Property<std::vector<std::string>> m_barPresVolumes{this, "BarrelPresVolumes"};
    Gaudi::Property<std::vector<std::string>> m_barVolumes{this, "BarrelVolumes"};
    Gaudi::Property<std::vector<std::string>> m_ECCryVolumes{this, "ECCryVolumes"};
    Gaudi::Property<std::vector<std::string>> m_ECCryLArVolumes{this, "ECCryLArVolumes"};
    Gaudi::Property<std::vector<std::string>> m_ECCryMixVolumes{this, "ECCryMixVolumes"};
    Gaudi::Property<std::vector<std::string>> m_ECSupportVolumes{this, "ECSupportVolumes"};
    Gaudi::Property<std::vector<std::string>> m_HECWheelVolumes{this, "HECWheelVolumes"};

    ServiceHandle<ILArCalibCalculatorSvc> m_embccalc{this, "EMBCryoCalibrationCalculator"
      , "BarrelCryostatCalibrationCalculator"}; //BarrelCryostat::CalibrationCalculator()
    ServiceHandle<ILArCalibCalculatorSvc> m_embclarcalc{this, "EMBCryoLArCalibrationCalculator"
      , "BarrelCryostatCalibrationLArCalculator"}; //BarrelCryostat::CalibrationLArCalculator()
    ServiceHandle<ILArCalibCalculatorSvc> m_mixcalc{this, "EMBCryoMixCalibrationCalculator"
      , "BarrelCryostatCalibrationMixedCalculator"}; //BarrelCryostat::CalibrationMixedCalculator()
    ServiceHandle<ILArCalibCalculatorSvc> m_dmcalc{this, "DMCalibrationCalculator"
      , "DMCalibrationCalculator"}; //DM::CalibrationCalculator()
    ServiceHandle<ILArCalibCalculatorSvc> m_embpscalc{this, "EMBPSCalibrationCalculator"
      , "BarrelPresamplerCalibrationCalculator"}; //BarrelPresampler::CalibrationCalculator()
    ServiceHandle<ILArCalibCalculatorSvc> m_embcalc{this, "EMBCalibrationCalculator"
      , "BarrelCalibrationCalculator"}; //Barrel::CalibrationCalculator()
    ServiceHandle<ILArCalibCalculatorSvc> m_emeccalc{this, "ECCryoCalibrationCalculator"
      , "EndcapCryostatCalibrationCalculator"}; //EndcapCryostat::CalibrationCalculator()
    ServiceHandle<ILArCalibCalculatorSvc> m_emecclarcalc{this, "ECCryoLArCalibrationCalculator"
      , "EndcapCryostatCalibrationLArCalculator"}; //EndcapCryostat::CalibrationLArCalculator()
    ServiceHandle<ILArCalibCalculatorSvc> m_ememixcalc{this, "ECCryoMixCalibrationCalculator"
      , "EndcapCryostatCalibrationMixedCalculator"}; //EndcapCryostat::CalibrationMixedCalculator()
    ServiceHandle<ILArCalibCalculatorSvc> m_emesupcalc{this, "EMECSuppCalibrationCalculator"
      , "EMECSupportCalibrationCalculator"}; //EMECSupportCalibrationCalculator()
    ServiceHandle<ILArCalibCalculatorSvc> m_heccalc{this, "HECWheelDeadCalculator"
      , "HECCalibrationWheelDeadCalculator"}; //HEC::LArHECCalibrationWheelCalculator(HEC::kWheelDead)
    ServiceHandle<ILArCalibCalculatorSvc> m_defcalc{this, "DefaultCalibrationCalculator"
      , "CalibrationDefaultCalculator"}; //CalibrationDefaultCalculator()
  };
} // namespace LArG4
#endif
