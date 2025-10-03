/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_FASTCALOSIM_FCS_STEPINFOSD_H
#define ISF_FASTCALOSIM_FCS_STEPINFOSD_H

#include <map>
#include <vector>

#include "CLHEP/Units/SystemOfUnits.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "CaloIdentifier/CaloCell_ID.h"  // For CaloCell_ID::CaloSample
#include "CxxUtils/CachedPointer.h"
#include "G4VSensitiveDetector.hh"
#include "ISF_FastCaloSimEvent/FCS_StepInfoCollection.h"
#include "LArG4Code/ILArCalculatorSvc.h"
#include "LArG4Code/LArG4Identifier.h"
#include "LArSimEvent/LArHit.h"
#include "TileG4Interfaces/ITileCalculator.h"

// Forward declarations
class G4Step;
class G4TouchableHistory;

class LArEM_ID;
class LArFCAL_ID;
class LArHEC_ID;
class TileID;

class LArHitContainer;

class StoreGateSvc;

namespace FCS_Param {

struct Config {
  /** Helper to keep the same verbosity everywhere */
  int verboseLevel = 0;
  bool shift_lar_subhit = true;
  bool shorten_lar_step = false;
  double substpsize = 0.2 * CLHEP::mm;  // size of splitting into substeps
                                        // before calling the calculators.

  // Merging properties
  std::vector<double> m_maxRadiusLateral{
      24, 5.};  //!< property, see @link LArG4GenShowerLib::LArG4GenShowerLib
                //!< @endlink
  std::vector<double> m_maxRadiusLongitudinal{
      24, 5.};  //!< property, see @link LArG4GenShowerLib::LArG4GenShowerLib
                //!< @endlink

  double m_maxTime = 25.;
  double m_maxTimeLAr = 25.;
  double m_maxTimeHEC = 25.;
  double m_maxTimeFCAL = 25.;
  double m_maxTimeTile = 25.;

  ILArCalculatorSvc* m_LArCalculator = nullptr;
  ITileCalculator* m_TileCalculator = nullptr;
};

}  // namespace FCS_Param

/// @class FCS_StepInfoSD
/// @brief Common sensitive detector class for LAr systems.
///
/// This SD implementation saves the standard LArHits.
/// See LArG4CalibSD for an SD that handles calibration hits.
///
class FCS_StepInfoSD : public G4VSensitiveDetector {
 public:
  /// Constructor
  FCS_StepInfoSD(G4String a_name, const FCS_Param::Config& config);

  /// Main processing method
  virtual G4bool ProcessHits(G4Step* a_step, G4TouchableHistory*) override;

  /// End of athena event processing
  void EndOfAthenaEvent(
      ISF_FCS_Parametrization::FCS_StepInfoCollection* hitContnainer);

  /// Sets the ID helper pointers
  void setupHelpers(const LArEM_ID* EM, const LArFCAL_ID* FCAL,
                    const LArHEC_ID* HEC, const TileID* tile) {
    m_larEmID = EM;
    m_larFcalID = FCAL;
    m_larHecID = HEC;
    m_tileID = tile;
  }

 protected:
  /// Keep a map instead of trying to keep the full vector.
  /// At the end of the event we'll push the map back into the
  /// FCS_StepInfoCollection in StoreGate.
  void getCaloDDManager();
  void update_map(const CLHEP::Hep3Vector& l_vec,
                  const Identifier& l_identifier, double l_energy,
                  double l_time, bool l_valid, int l_detector);
  FCS_Param::Config m_config;
  /// Pointers to the identifier helpers
  const LArEM_ID* m_larEmID{nullptr};
  const LArFCAL_ID* m_larFcalID{nullptr};
  const LArHEC_ID* m_larHecID{nullptr};
  const TileID* m_tileID{nullptr};
  CxxUtils::CachedPointer<const CaloDetDescrManager> m_calo_dd_man;
  std::map<Identifier, std::vector<ISF_FCS_Parametrization::FCS_StepInfo*>*>
      m_hit_map;

 private:
  ///
  double getMaxTime(const CaloCell_ID::CaloSample& layer) const;
};

#endif  // ISF_FASTCALOSIM_FCS_STEPINFOSD_H
