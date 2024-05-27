/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "ZDC_G4CalibSD.h"
#include "CaloSimEvent/CaloCalibrationHitContainer.h"
#include "CaloSimEvent/CaloCalibrationHit.h"
#include "CaloG4Sim/SimulationEnergies.h"
#include "CaloG4Sim/EscapedEnergyRegistry.h"
#include "G4RunManager.hh"
#include "G4Step.hh"
#include "MCTruth/AtlasG4EventUserInfo.h"

ZDC_G4CalibSD::ZDC_G4CalibSD(const G4String &a_name, const G4String& hitCollectionName, bool doPID)
    : G4VSensitiveDetector(a_name), m_HitColl(hitCollectionName), m_numberInvalidHits(0), m_doPID(doPID)
{
  m_simulationEnergies = new CaloG4::SimulationEnergies();
}

ZDC_G4CalibSD::~ZDC_G4CalibSD()
{
  if (verboseLevel > 5 && m_numberInvalidHits > 0)
  {
    G4cout << "Destructor: Sensitive Detector <" << SensitiveDetectorName << "> had " << m_numberInvalidHits
           << " G4Step energy deposits outside the region determined by its Calculator." << G4endl;
  }
  delete m_simulationEnergies;
}

G4bool ZDC_G4CalibSD::ProcessHits(G4Step *a_step, G4TouchableHistory *)
{
  // If there's no energy, there's no hit.  (Aside: Isn't this energy
  // the same as the energy from the calculator?  Not necessarily.
  // The calculator may include detector effects such as
  // charge-collection which are not modeled by Geant4.)
  if (a_step->GetTotalEnergyDeposit() == 0.)
    return false;
  
  // Convert the G4Step into (eta,phi,sampling).
  // Check that hit was valid.  (It might be invalid if, for example,
  // it occurred outside the sensitive region.  If such a thing
  // happens, it means that the geometry definitions in the
  // detector-construction routine and the calculator do not agree.)
  
  m_energies.clear();
  // classify different types of deposits (0: EM, 1: Non-EM, 2: Invisible, 3: Escaped)
 
  m_simulationEnergies->Energies(a_step, m_energies);

  // identifier needed to specify particular volme we're in. Used in HitCollection to make sure we don't have double hits
  Identifier id;
  id = a_step->GetPreStepPoint()->GetPhysicalVolume()->GetCopyNo();

  // build calibHit. check if we've had a hit in this cell already. if we havent add it to the set of cells. If we have add energies to existing energies. ie can't distinguish b/w different hits in single cell so must integrate all this information
  return SimpleHit (id, m_energies );
}


G4bool ZDC_G4CalibSD::SimpleHit(const Identifier& id, const std::vector<double>& energies )
{

  // retreive particle ID
  unsigned int particleID = 0;
  if( m_doPID ) {
    AtlasG4EventUserInfo * atlasG4EvtUserInfo = dynamic_cast<AtlasG4EventUserInfo*>(G4RunManager::GetRunManager()->GetCurrentEvent()->GetUserInformation());
    if (atlasG4EvtUserInfo) particleID = HepMC::barcode(atlasG4EvtUserInfo->GetCurrentPrimary()); // FIXME Barcode-based
  }


  // Reject cases where invisible energy calculation produced values
  // of order 10e-12 instead of 0 due to rounding errors in that
  // calculation, or general cases where the energy deposits are
  // trivially small.
  if (energies[0] + energies[1] + energies[3] < 0.001 * CLHEP::eV && std::abs(energies[2]) < 0.001 * CLHEP::eV)
  {
    return true;
  }

  // Build the hit.
  CaloCalibrationHit *hit = new CaloCalibrationHit(id,
                                                   energies[0],
                                                   energies[1],
                                                   energies[2],
                                                   energies[3],
                                                   particleID);

  //Get the hash for this volume to keep track of hits
  uint32_t hash = id.get_identifier32().get_compact();

  std::map<uint32_t,CaloCalibrationHit*>::iterator it = m_hitMap.find(hash);

  if(it == m_hitMap.end()){
    //This is a new hit, insert it
    m_hitMap.insert(std::pair<uint32_t,CaloCalibrationHit*>(hash,hit));
  }else{
    //Add this hit to the existing one for this volume
    it->second->Add(hit);
    delete hit;
  }
  
  return true;
}

// Something special has happened (probably the detection of escaped
// energy in CaloG4Sim/SimulationEnergies).  We have to bypass the
// regular sensitive-detector processing.  Determine the identifier
// (and only the identifier) using the calculator, then built a hit
// with that identifier and the energies in the vector.

G4bool ZDC_G4CalibSD::SpecialHit(G4Step *a_step,
                                 const std::vector<G4double> &a_energies)
{
  // If we can't get the identifier, something is wrong.
  Identifier id;
  id = a_step->GetPreStepPoint()->GetPhysicalVolume()->GetCopyNo();

  return SimpleHit(id, a_energies);
}

void ZDC_G4CalibSD::EndOfAthenaEvent()
{

  //Move the hits from the hit set to the hit container
  if (!m_HitColl.isValid())
    m_HitColl = std::make_unique<CaloCalibrationHitContainer>(m_HitColl.name());

  // Loop through the hits...
  for (auto hit : m_hitMap)
  {
    m_HitColl->push_back(hit.second);
  } // End of loop over hits

  // Clean up
  m_hitMap.clear();
}
