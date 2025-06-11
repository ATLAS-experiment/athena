/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARG4CODE_LARG4SDTOOL_H
#define LARG4CODE_LARG4SDTOOL_H

// Base class
#include "G4AtlasTools/SensitiveDetectorBase.h"
// Units!
#include "CLHEP/Units/SystemOfUnits.h"

#include <string>
#include <vector>
class G4VSensitiveDetector;

// For the identifier helpers
class LArEM_ID;
class LArFCAL_ID;
class LArHEC_ID;
class CaloDM_ID;
class LArG4SimpleSD;
class LArG4CalibSD;

/// @class LArG4SDTool
/// @brief A base class for tools that manage LAr SDs.
///
/// This tool class adds functionality on top of SensitiveDetectorBase which
/// is specific to the LAr setups.
///
/// NOTE: this design is not yet good for multi-threading in the existing
/// ATLAS MT simulation model. Thus, there may be some refactoring needed.
///
class LArG4SDTool : public SensitiveDetectorBase
{
 public:
  /// Constructor
  LArG4SDTool(const std::string& type, const std::string& name, const IInterface *parent);

  /// Destructor
  virtual ~LArG4SDTool() = default;

  StatusCode initialize() override final;

  virtual StatusCode initializeCalculators(){ return StatusCode::SUCCESS; }

  /// Helper method to pass the ID helper pointers to the SDs.
  void setupHelpers( LArG4SimpleSD* ) const;
  /// Overload of the above method for calib SDs.
  void setupHelpers( LArG4CalibSD* ) const;

 protected:

  G4VSensitiveDetector* makeSD() const override final { return nullptr; }

  /// Are we set up to run with PID hits?
  Gaudi::Property<G4bool> m_doPID{this, "ParticleID", false};
  /// What time binning type for regular hits?
  Gaudi::Property<std::string> m_timeBinType{this, "TimeBinType", "Default"};
  /// What time bin width for regular hits?
  Gaudi::Property<float> m_timeBinWidth{this, "TimeBinWidth", 2.5*CLHEP::ns};
  /// Is there going to be a fast simulation coming into this SD?
  Gaudi::Property<G4bool> m_useFrozenShowers{this, "UseFrozenShowers", false};

  /// Method actually doing the work to assign all of these SDs
  /// FIXME: this argument type is nasty:
  ///  - non-const ptr to list of strings
  void setupAllSDs(const std::map<G4VSensitiveDetector*,std::vector<std::string>*>& configuration) const;

  /// Pointers to the identifier helpers
  const LArEM_ID*       m_larEmID{nullptr};
  const LArFCAL_ID*     m_larFcalID{nullptr};
  const LArHEC_ID*      m_larHecID{nullptr};
  const CaloDM_ID*      m_caloDmID{nullptr};

  /// Helper function for matching strings with wildcards
  bool match(const char *first, const char * second) const;

};

#endif
