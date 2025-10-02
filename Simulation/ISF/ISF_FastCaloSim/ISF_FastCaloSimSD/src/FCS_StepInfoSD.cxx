/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "FCS_StepInfoSD.h"

#include <utility>

#include "CaloDetDescr/CaloDetDescrElement.h"
#include "CaloIdentifier/CaloIdManager.h"
#include "CaloIdentifier/LArEM_ID.h"
#include "CaloIdentifier/LArFCAL_ID.h"
#include "CaloIdentifier/LArHEC_ID.h"
#include "CaloIdentifier/LArID_Exception.h"
#include "CaloIdentifier/TileID.h"
#include "G4Step.hh"
#include "G4ThreeVector.hh"
#include "StoreGate/ReadCondHandle.h"
#include "StoreGate/ReadCondHandleKey.h"

FCS_StepInfoSD::FCS_StepInfoSD(G4String a_name, const FCS_Param::Config& config)
    : G4VSensitiveDetector(std::move(a_name)),
      m_config(config),
      m_calo_dd_man(nullptr) {}

G4bool FCS_StepInfoSD::ProcessHits(G4Step*, G4TouchableHistory*) {
  G4ExceptionDescription description;
  description << "ProcessHits: Base class method should not be called!!!";
  G4Exception("FCS_StepInfoSD", "FCSBadCall", FatalException, description);
  abort();
  return false;
}

inline double FCS_StepInfoSD::getMaxTime(
    const CaloCell_ID::CaloSample& layer) const {
  /// NB The result of this function should actually be constant for each SD
  if (layer >= CaloCell_ID::PreSamplerB && layer <= CaloCell_ID::EME3) {
    return m_config.m_maxTimeLAr;
  }
  if (layer >= CaloCell_ID::HEC0 && layer <= CaloCell_ID::HEC3) {
    return m_config.m_maxTimeHEC;
  }
  if (layer >= CaloCell_ID::FCAL0 && layer <= CaloCell_ID::FCAL2) {
    return m_config.m_maxTimeFCAL;
  }
  return m_config.m_maxTime;
}

void FCS_StepInfoSD::getCaloDDManager() {
  SG::ReadCondHandleKey<CaloDetDescrManager> caloMgrKey{"CaloDetDescrManager"};
  if (caloMgrKey.initialize().isFailure()) {
    G4ExceptionDescription description;
    description << "Failed to get CaloDetDescrManager!";
    G4Exception("FCS_StepInfoSD", "FCSBadCall", FatalException, description);
    abort();
  }
  SG::ReadCondHandle<CaloDetDescrManager> caloMgr(
      caloMgrKey, Gaudi::Hive::currentContext());
  m_calo_dd_man.set(*caloMgr);
}

void FCS_StepInfoSD::update_map(const CLHEP::Hep3Vector& l_vec,
                                const Identifier& l_identifier, double l_energy,
                                double l_time, bool l_valid,
                                int l_detector)  // TODO: is the "&" needed?
{
  // NB l_identifier refers to:
  // - the cell identifier for LAr
  // - the PMT identifier for Tile

  // Drop any hits that don't have a good identifier attached
  if (!m_calo_dd_man.get()->get_element(l_identifier)) {
    if (m_config.verboseLevel > 4) {
      G4cout << this->GetName() << " DEBUG update_map: bad identifier: "
             << l_identifier.getString() << " skipping this hit." << G4endl;
    }
    return;
  }

  auto map_item = m_hit_map.find(l_identifier);
  if (map_item == m_hit_map.end()) {
    m_hit_map[l_identifier] =
        new std::vector<ISF_FCS_Parametrization::FCS_StepInfo*>;
    m_hit_map[l_identifier]->reserve(200);
    m_hit_map[l_identifier]->push_back(
        new ISF_FCS_Parametrization::FCS_StepInfo(l_vec, l_identifier, l_energy,
                                                  l_time, l_valid, l_detector));
  } else {

    // Get the appropriate merging limits
    const CaloCell_ID::CaloSample& layer =
        m_calo_dd_man.get()->get_element(l_identifier)->getSampling();

    double timeWindow = m_config.m_maxTime;
    const double distWinLong = m_config.m_maxRadiusLongitudinal.at(layer);
    const double distWinLat = m_config.m_maxRadiusLateral.at(layer);

    const double tsame(this->getMaxTime(layer));
    bool match = false;
    for (auto* map_it : *map_item->second) {
      // Time check ... both a global flag and a check on the layer
      const double delta_t = std::fabs(map_it->time() - l_time);
      if (delta_t >= tsame) {
        continue;
      }
      if (delta_t >= timeWindow) {
        continue;
      }

      // Distance check
      const CLHEP::Hep3Vector& currentPosition = map_it->position();
      const double currentPosition_mag = currentPosition.mag();
      const double proj_longitudinal =
          currentPosition.dot(l_vec) / currentPosition_mag;
      const double delta_longitudinal = currentPosition_mag - proj_longitudinal;
      if (std::fabs(delta_longitudinal) >= distWinLong) {
        continue;
      }

      // Lateral distance check
      double delta_lateral_2 = l_vec.mag2() - proj_longitudinal * proj_longitudinal;
      if (delta_lateral_2 < 0) {
        delta_lateral_2 = 0;  // Avoid negative square root
      }
      const double delta_lateral =
          std::sqrt(delta_lateral_2);
      if (delta_lateral >= distWinLat) {
        continue;
      }

      // Found a match.  Make a temporary that will be deleted!
      const ISF_FCS_Parametrization::FCS_StepInfo my_info(
          l_vec, l_identifier, l_energy, l_time, l_valid, l_detector);
      *map_it += my_info;
      match = true;
      break;
    }  // End of search for match in time and space
    if (!match) {
      map_item->second->push_back(new ISF_FCS_Parametrization::FCS_StepInfo(
          l_vec, l_identifier, l_energy, l_time, l_valid, l_detector));
    }  // Didn't match
  }  // ID already in the map
  return;
}  // That's it for updating the map!

void FCS_StepInfoSD::EndOfAthenaEvent(
    ISF_FCS_Parametrization::FCS_StepInfoCollection* hitContainer) {
  // Unpack map into vector
  for (auto it : m_hit_map) {
    for (auto* a_s : *it.second) {
      // Giving away ownership of the objects!
      hitContainer->push_back(a_s);
    }
    it.second->clear();
    delete it.second;
  }  // Vector of IDs in the map
  m_hit_map.clear();
  if (m_config.verboseLevel > 4) {
    G4cout << this->GetName()
           << " DEBUG EndOfAthenaEvent: After initial cleanup, N="
           << hitContainer->size() << G4endl;
  }
  return;
}
