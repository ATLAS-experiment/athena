/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARG4CODE_LARG4SIMPLESD_H
#define LARG4CODE_LARG4SIMPLESD_H

#include "G4VSensitiveDetector.hh"

#include "LArG4Code/LArG4Identifier.h"

#include "LArSimEvent/LArHit.h"
#include "CLHEP/Units/SystemOfUnits.h"
#include <gtest/gtest_prod.h>

// Forward declarations
class LArEM_ID;
class LArFCAL_ID;
class LArHEC_ID;

class ILArCalculatorSvc;
class LArHitContainerBuilder;
class G4HCofThisEvent;

class StoreGateSvc;


/// @class LArG4SimpleSD
/// @brief Common sensitive detector class for LAr systems.
///
/// This SD implementation saves the standard LArHits.
/// See LArG4CalibSD for an SD that handles calibration hits.
/// Event state is owned by `LArHitContainerBuilder`; the SD registers its
/// regular-SD partition during `Initialize()` and then writes hits through the
/// builder for the current Athena event.
///
class LArG4SimpleSD : public G4VSensitiveDetector
{
FRIEND_TEST( LArG4SimpleSDtest, ProcessHits );
FRIEND_TEST( LArG4SimpleSDtest, setupHelpers );
FRIEND_TEST( LArG4SimpleSDtest, getTimeBin );
FRIEND_TEST( LArG4SimpleSDtest, SimpleHit );
FRIEND_TEST( LArG4SimpleSDtest, ConvertID );
public:

  enum LArHitTimeBins
  {
      HitTimeBinDefault = 0,
      HitTimeBinUniform = 1
  };

  /// Constructor
  LArG4SimpleSD(G4String a_name, ILArCalculatorSvc* calc,
                std::string hitCollectionName,
                const std::string& type="Default",
                const float width=2.5*CLHEP::ns);

  /// Alternative constructor, particularly for fast simulations.
  LArG4SimpleSD(G4String a_name, StoreGateSvc* detStore,
                std::string hitCollectionName);

  /// Destructor
  virtual ~LArG4SimpleSD();

  /// Register this regular SD with the event-owned builder.
  void Initialize(G4HCofThisEvent*) override;

  /// Main processing method
  G4bool ProcessHits(G4Step* a_step, G4TouchableHistory*) override;

  /// First method translates to this - also for fast sims
  G4bool SimpleHit( const LArG4Identifier& lar_id , G4double time , G4double energy );

  /// Sets the ID helper pointers
  void setupHelpers( const LArEM_ID* EM ,
                     const LArFCAL_ID* FCAL ,
                     const LArHEC_ID* HEC ) {
    m_larEmID = EM;
    m_larFcalID = FCAL;
    m_larHecID = HEC;
  }

  /// Helper function for making "real" identifiers from LArG4Identifiers
  Identifier ConvertID(const LArG4Identifier& a_ident) const;

protected:
  /// Helper method for time info
  G4int getTimeBin(G4double time) const;

  /// Member variable - the calculator we'll use
  ILArCalculatorSvc * m_calculator;

  /// Count the number of invalid hits.
  G4int m_numberInvalidHits;

  // Two types of LAr hit time binning
  // 1. 'Default'
  //
  //     All negative times to the bin 0
  //     0 <= time < 10 by 2.5ns
  //     10 <= time < 50 by 10ns
  //     50 <= time < 100 by 25ns
  //     All others to the bin 14
  //
  //
  // 2. 'Uniform'
  //
  // Old style time binning by 2.5ns
  LArG4SimpleSD::LArHitTimeBins m_timeBinType;

  /// Width of the time bins for summing hits - for the uniform binning
  G4float m_timeBinWidth;

  /// Pointers to the identifier helpers
  const LArEM_ID*       m_larEmID;
  const LArFCAL_ID*     m_larFcalID;
  const LArHEC_ID*      m_larHecID;

  LArHitContainerBuilder* getHitContainer() const;

private:
  std::string m_hitCollectionName;
  std::string m_hitSourceName;

};

#endif
