/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARG4CALIBSD_H
#define LARG4CALIBSD_H

#include "G4VSensitiveDetector.hh"

#include "LArG4Code/LArCalibrationHitContainerBuilder.h"
#include "LArG4Code/LArG4Identifier.h"
#include "CaloSimEvent/CaloCalibrationHit.h"
#include <gtest/gtest_prod.h>

#include <vector>

// Forward declarations
class G4Step;
class G4Track;
class G4HCofThisEvent;

class LArEM_ID;
class LArFCAL_ID;
class LArHEC_ID;
class CaloDM_ID;
class AtlasDetectorID;

class ILArCalibCalculatorSvc; 

/// @class LArG4CalibSD
/// @brief A specialized SD class for saving LAr calibration hits.
///
/// This SD implementation saves CaloCalibrationHit objects.
/// See LArG4SimpleSD for an SD that handles the standard LArHits.
/// Event state is owned by the calibration builder containers; the SD registers
/// its regular-SD partitions during `Initialize()` and then writes through the
/// builders for the current Athena event.
///
class LArG4CalibSD : public G4VSensitiveDetector
{
FRIEND_TEST( LArG4CalibSDtest, ProcessHits );
FRIEND_TEST( LArG4CalibSDtest, setupHelpers );
FRIEND_TEST( LArG4CalibSDtest, addDetectorHelper );
FRIEND_TEST( LArG4CalibSDtest, SpecialHit );
FRIEND_TEST( LArG4CalibSDtest, SimpleHit );
FRIEND_TEST( LArG4CalibSDtest, ConvertID );
public:

  /// Constructor
  LArG4CalibSD(G4String a_name, ILArCalibCalculatorSvc* calc,
               std::string hitCollectionName,
               std::string deadHitCollectionName,
               std::string srHitCollectionName,
               bool doPID=false);

  /// Destructor
  virtual ~LArG4CalibSD();

  /// Register this regular SD with the event-owned builders.
  void Initialize(G4HCofThisEvent*) override;

  /// Main processing method
  G4bool ProcessHits(G4Step* a_step,G4TouchableHistory*) override;

  /// Sets the ID helper pointers
  void setupHelpers( const LArEM_ID* EM ,
                     const LArFCAL_ID* FCAL ,
                     const LArHEC_ID* HEC ,
                     const CaloDM_ID* caloDm ) {
    m_larEmID = EM;
    m_larFcalID = FCAL;
    m_larHecID = HEC;
    m_caloDmID = caloDm;
  }

  void addDetectorHelper( const AtlasDetectorID* id_helper) { m_id_helper=id_helper; }

  /// For other classes that need to call into us...
  G4bool SpecialHit(G4Step* a_step, const std::vector<G4double>& a_energies);

protected:
  /// Constructs the calibration hit and saves it to the appropriate builder
  G4bool SimpleHit(const LArG4Identifier& a_ident,
                   const std::vector<double>& energies,
                   bool deadMaterialHit = false,
                   const G4Track* track = nullptr);

  G4bool SrHit(const LArG4Identifier& a_ident, const LArG4Identifier& sr_id,
               const std::vector<double>& energies,
               const G4Track* track = nullptr);
  /// Member variable - the calculator we'll use
  ILArCalibCalculatorSvc * m_calculator;

  /// Count the number of invalid hits.
  G4int m_numberInvalidHits;

  /// Are we set up to run with PID hits?
  G4bool m_doPID;

  /// Helper function for making "real" identifiers from LArG4Identifiers
  Identifier ConvertID(const LArG4Identifier& a_ident) const;

  Identifier ConvertSRID(const LArG4Identifier& sr_id, const LArG4Identifier& lr_id) const;
  /// Pointers to the identifier helpers
  const LArEM_ID*       m_larEmID;
  const LArFCAL_ID*     m_larFcalID;
  const LArHEC_ID*      m_larHecID;
  const CaloDM_ID*      m_caloDmID;
  const AtlasDetectorID* m_id_helper;

  LArCalibrationHitContainerBuilder* getCalibrationHits(bool deadMaterialHit) const;
  LArSrCalibrationHitContainerBuilder* getSrCalibrationHits() const;

private:
  std::string m_hitCollectionName;
  std::string m_deadHitCollectionName;
  std::string m_srHitCollectionName;
  std::string m_hitSourceName;
};

#endif
