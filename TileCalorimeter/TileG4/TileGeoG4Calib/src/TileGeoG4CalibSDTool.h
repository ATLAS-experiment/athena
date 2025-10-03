/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//************************************************************
//
// Class TileGeoG4CalibSDTool.
// AthTool class for holding the Calibration Sensitive Detector for TileCal simulation
//
//************************************************************

#ifndef TILEGEOG4CALIB_TILEGEOG4CALIBSDTOOL_H
#define TILEGEOG4CALIB_TILEGEOG4CALIBSDTOOL_H

#include "G4AtlasTools/SensitiveDetectorBase.h"

// Uses
#include <string>

#include "TileG4Interfaces/ITileCalculator.h"

// Members
class G4VSensitiveDetector;

class TileGeoG4CalibSDTool: public SensitiveDetectorBase {
public:
  /// Constructor
  TileGeoG4CalibSDTool(const std::string& type, const std::string& name, const IInterface *parent);
  /// Default constructor is fine

  ///
  virtual StatusCode initialize() override final;
  
  virtual StatusCode SetupEvent(HitCollectionMap&) override final;

  /// End of an athena event
  virtual StatusCode Gather(HitCollectionMap&) override final;

protected:
  /// Make me an SD!
  virtual G4VSensitiveDetector* makeSD() const override final;

private:
  /// Calculator Service
  ServiceHandle<ITileCalculator> m_tileCalculator{this, "TileCalculator", "TileGeoG4SDCalc"};

  std::string m_tileHits{};
  std::string m_tileActiveCellCalibHits{};
  std::string m_tileInactiveCellCalibHits{};
  std::string m_tileDeadMaterialCalibHits{};

};

#endif // TILEGEOG4CALIB_TILEGEOG4CALIBSDTOOL_H

