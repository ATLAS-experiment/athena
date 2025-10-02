/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARG4SD_EMECSDTOOL_H
#define LARG4SD_EMECSDTOOL_H

// System includes
#include <string>
#include <vector>

// Project includes
#include "LArG4Code/SimpleSDTool.h"
#include "LArG4Code/ILArCalculatorSvc.h"

namespace LArG4
{

  /// @class EMECSDTool
  /// @brief SD tool which manages EM endcap sensitive detectors.
  ///
  /// NOTE: this design is in flux, migrating to be more multi-threading-friendly
  ///
  class EMECSDTool : public SimpleSDTool
  {
    public:

    /// Constructor
    EMECSDTool(const std::string& type, const std::string& name,
	       const IInterface* parent);

  private:
    /// Initialize Calculator Services
    StatusCode initializeCalculators() override final;

    /// Create the SD wrapper for current worker thread
    G4VSensitiveDetector* makeSD() const override final;

    /// @name List of volumes for each SD and the corresponding SD
    /// @{
    Gaudi::Property<std::vector<std::string>> m_posIWVolumes{this, "PosIWVolumes"};
    Gaudi::Property<std::vector<std::string>> m_negIWVolumes{this, "NegIWVolumes"};
    Gaudi::Property<std::vector<std::string>> m_posOWVolumes{this, "PosOWVolumes"};
    Gaudi::Property<std::vector<std::string>> m_negOWVolumes{this, "NegOWVolumes"};
    Gaudi::Property<std::vector<std::string>> m_presVolumes{this, "PresVolumes"};
    Gaudi::Property<std::vector<std::string>> m_posBOBVolumes{this, "PosBOBarretteVolumes"};
    Gaudi::Property<std::vector<std::string>> m_negBOBVolumes{this, "NegBOBarretteVolumes"};
    /// @}
    
    ServiceHandle<ILArCalculatorSvc> m_emepiwcalc{this, "EMECPosIWCalculator"
      , "EMECPosInnerWheelCalculator"}; //EnergyCalculator(LArG4::InnerAbsorberWheel, LArG4::EMEC_ECOR_ROPT, 1)
    ServiceHandle<ILArCalculatorSvc> m_emeniwcalc{this, "EMECNegIWCalculator"
      , "EMECNegInnerWheelCalculator"}; //EC::EnergyCalculator(LArWheelCalculator::InnerAbsorberWheel, LArG4::EMEC_ECOR_ROPT, -1),
    ServiceHandle<ILArCalculatorSvc> m_emepowcalc{this, "EMECPosOWCalculator"
      , "EMECPosOuterWheelCalculator"}; //EC::EnergyCalculator(LArWheelCalculator::OuterAbsorberWheel, EC::EnergyCalculator::EMEC_ECOR_ROPT, 1),
    ServiceHandle<ILArCalculatorSvc> m_emenowcalc{this, "EMECNegOWCalculator"
      , "EMECNegOuterWheelCalculator"}; //EC::EnergyCalculator(LArWheelCalculator::OuterAbsorberWheel, EC::EnergyCalculator::EMEC_ECOR_ROPT, -1),
    ServiceHandle<ILArCalculatorSvc> m_emepscalc{this, "EMECPSCalculator"
      , "EMECPresamplerCalculator"}; //LArEndcapPresamplerCalculator::GetCalculator()
    ServiceHandle<ILArCalculatorSvc> m_emepobarcalc{this, "EMECPosBOBCalculator"
      , "EMECPosBackOuterBarretteCalculator"}; //EC::EnergyCalculator(LArWheelCalculator::BackOuterBarretteWheel) / Pos
    ServiceHandle<ILArCalculatorSvc> m_emenobarcalc{this, "EMECNegBOBCalculator"
      , "EMECNegBackOuterBarretteCalculator"}; //EC::EnergyCalculator(LArWheelCalculator::BackOuterBarretteWheel) / Neg
  }; // class EMECSDTool
} // namespace LArG4
#endif
