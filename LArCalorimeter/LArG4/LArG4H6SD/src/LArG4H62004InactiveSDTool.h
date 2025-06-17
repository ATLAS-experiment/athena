/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARG4H62004INACTIVESDTOOL_H
#define LARG4H62004INACTIVESDTOOL_H

#include "LArG4Code/LArG4SDTool.h"
#include <string>
#include <vector>

#include "StoreGate/WriteHandle.h"
#include "CaloSimEvent/CaloCalibrationHitContainer.h"
#include "LArG4Code/ILArCalibCalculatorSvc.h"

class LArG4CalibSD;

/// DEPRECATED AND WILL BE REMOVED.
/// Please see LArG4::H62004InactiveSDTool instead.
///
class LArG4H62004InactiveSDTool : public LArG4SDTool
{
 public:
  // Constructor
  LArG4H62004InactiveSDTool(const std::string& type, const std::string& name, const IInterface *parent);

  // Destructor
  virtual ~LArG4H62004InactiveSDTool() = default;

  virtual StatusCode initializeCalculators() override final;

  // Method in which all the SDs are created and assigned to the relevant volumes
  StatusCode initializeSD() override final;

  // Calls down to all the SDs to get them to pack their hits into a central collection
  StatusCode Gather() override final;

 private:
  // The actual hit container - here because the base class is for both calib and standard SD tools
  SG::WriteHandle<CaloCalibrationHitContainer> m_HitColl;

  ServiceHandle<ILArCalibCalculatorSvc>m_emepiwcalc {this, "EMECPosIWCalibrationCalculator"
    , "EMECPosInnerWheelCalibrationCalculator"};
  ServiceHandle<ILArCalibCalculatorSvc>m_heccalc {this, "HECWheelInactiveCalculator"
    , "LocalCalibrationInactiveCalculator"};
  ServiceHandle<ILArCalibCalculatorSvc>m_fcal1calc {this, "FCAL1CalibCalculator"
    , "LArFCAL1H62004CalibCalculator"};
  ServiceHandle<ILArCalibCalculatorSvc>m_fcal2calc {this, "FCAL2CalibCalculator"
    , "LArFCAL2H62004CalibCalculator"};

  // The list of volumes and the corresponding SDs
  LArG4CalibSD* m_emecSD {nullptr};
  LArG4CalibSD* m_hecSD {nullptr};
  LArG4CalibSD* m_fcal1SD {nullptr};
  LArG4CalibSD* m_fcal2SD {nullptr};
  Gaudi::Property<std::vector<std::string>> m_emecVolumes {this, "EMECVolumes"};
  Gaudi::Property<std::vector<std::string>> m_hecVolumes {this, "HECVolumes"};
  Gaudi::Property<std::vector<std::string>> m_fcal1Volumes {this, "FCAL1Volumes"};
  Gaudi::Property<std::vector<std::string>> m_fcal2Volumes {this, "FCAL2Volumes"};
};

#endif
