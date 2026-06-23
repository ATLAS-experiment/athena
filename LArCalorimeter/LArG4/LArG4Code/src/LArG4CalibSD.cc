/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <memory>
#include <utility>

#include "LArG4Code/LArG4CalibSD.h"

#include "LArG4Code/LArCalibrationHitContainerBuilder.h"
#include "LArG4Code/ILArCalibCalculatorSvc.h"
#include "CaloIdentifier/LArID_Exception.h"
#include "CaloIdentifier/LArEM_ID.h"
#include "CaloIdentifier/LArFCAL_ID.h"
#include "CaloIdentifier/LArHEC_ID.h"
#include "CaloIdentifier/CaloDM_ID.h"


#include "G4RunManager.hh"
#include "G4EventManager.hh"
#include "MCTruth/AtlasG4EventUserInfo.h"
#include "HitManagement/HitCollectionMap.h"
#include "AtlasDetDescr/AtlasDetectorID.h"
#include "G4Step.hh"

LArG4CalibSD::LArG4CalibSD(G4String a_name, ILArCalibCalculatorSvc* calc,
                           std::string hitCollectionName,
                           std::string deadHitCollectionName,
                           std::string srHitCollectionName,
                           bool doPID)
  : G4VSensitiveDetector(std::move(a_name))
  , m_calculator(calc)
  , m_numberInvalidHits(0)
  , m_doPID(doPID)
  , m_larEmID (nullptr)
  , m_larFcalID (nullptr)
  , m_larHecID (nullptr)
  , m_caloDmID (nullptr)
  , m_id_helper (nullptr)
  , m_hitCollectionName(std::move(hitCollectionName))
  , m_deadHitCollectionName(std::move(deadHitCollectionName))
  , m_srHitCollectionName(std::move(srHitCollectionName))
  , m_hitSourceName(SensitiveDetectorName)
{}

LArG4CalibSD::~LArG4CalibSD()
{
  if(verboseLevel>5 && m_numberInvalidHits>0) {
    G4cout << "Destructor: Sensitive Detector <" << SensitiveDetectorName << "> had " << m_numberInvalidHits
           << " G4Step energy deposits outside the region determined by its Calculator." << G4endl;
  }
}

void LArG4CalibSD::Initialize(G4HCofThisEvent*)
{
  if (auto* hitContainer = getCalibrationHits(false)) {
    // Register before any hits arrive so finalization follows SD setup order.
    hitContainer->RegisterSource(m_hitSourceName);
  }
  if (auto* deadHitContainer = getCalibrationHits(true)) {
    deadHitContainer->RegisterSource(m_hitSourceName);
  }
  if (auto* srHitContainer = getSrCalibrationHits()) {
    srHitContainer->RegisterSource(m_hitSourceName);
  }
}

G4bool LArG4CalibSD::ProcessHits(G4Step* a_step,G4TouchableHistory*)
{
  // If there's no energy, there's no hit.  (Aside: Isn't this energy
  // the same as the energy from the calculator?  Not necessarily.
  // The calculator may include detector effects such as
  // charge-collection which are not modeled by Geant4.)
  //if(a_step->GetTotalEnergyDeposit() == 0.) return false; //FIXME commenting this out to fix for ATLASSIM-3065

  // Convert the G4Step into (eta,phi,sampling).
  // Check that hit was valid.  (It might be invalid if, for example,
  // it occurred outside the sensitive region.  If such a thing
  // happens, it means that the geometry definitions in the
  // detector-construction routine and the calculator do not agree.)
  LArG4Identifier identifier;
  LArG4Identifier identifier_sr;
  std::vector<G4double>  energies;

  if (!(m_calculator->Process(a_step, identifier, identifier_sr, energies, LArG4::kEnergyAndID))) {
    return false;
  }

  // Process SR hit if provided
  // Probably want to add a flag to enable/disable this
  if(identifier_sr != LArG4Identifier()) {
      SrHit(identifier, identifier_sr, energies);
    }

  if(m_id_helper) {
    Identifier id = this->ConvertID( identifier );
    if(id.is_valid() && m_id_helper->is_lar_dm(id)) {
      return SimpleHit(identifier, energies, true);
    }
  }
  return SimpleHit(identifier, energies, false);
}

G4bool LArG4CalibSD::SimpleHit(const LArG4Identifier& a_ident,
                               const std::vector<double>& energies,
                               bool deadMaterialHit)
{

  // retreive particle ID
  int particleID{HepMC::UNDEFINED_ID};
  int particleUID{HepMC::UNDEFINED_ID};
  if( m_doPID ) {
    AtlasG4EventUserInfo * atlasG4EvtUserInfo = dynamic_cast<AtlasG4EventUserInfo*>(G4RunManager::GetRunManager()->GetCurrentEvent()->GetUserInformation());
    if (atlasG4EvtUserInfo) {
      particleID = HepMC::barcode(atlasG4EvtUserInfo->GetCurrentPrimaryGenParticle()); // FIXME Barcode-based
      particleUID = HepMC::uniqueID(atlasG4EvtUserInfo->GetCurrentPrimaryGenParticle());
    }
  }

  // Build the hit from the calculator results.
  Identifier id = ConvertID( a_ident );
  if (!id.is_valid()) return true;

  // Reject cases where invisible energy calculation produced values
  // of order 10e-12 instead of 0 due to rounding errors in that
  // calculation, or general cases where the energy deposits are
  // trivially small.
  if(energies[0]+energies[1]+energies[3] < 0.001*CLHEP::eV && std::abs(energies[2]) < 0.001*CLHEP::eV ) {
    return true;
  }

  auto* hitCollection = getCalibrationHits(deadMaterialHit);
  if (!hitCollection) {
    return false;
  }
  hitCollection->AddHit(m_hitSourceName,
                        std::make_unique<CaloCalibrationHit>(id,
                                                             energies[0],
                                                             energies[1],
                                                             energies[2],
                                                             energies[3],
                                                             particleID,
                                                             particleUID));

  return true;
}


// Something special has happened (probably the detection of escaped
// energy in CaloG4Sim/SimulationEnergies).  We have to bypass the
// regular sensitive-detector processing.  Determine the identifier
// (and only the identifier) using the calculator, then built a hit
// with that identifier and the energies in the vector.

G4bool LArG4CalibSD::SpecialHit(G4Step* a_step,
                                const std::vector<G4double>& a_energies)
{
  LArG4Identifier identifier;
  LArG4Identifier identifier_sr;
  std::vector<G4double>  vtmp;

  // If we can't calculate the identifier, something is wrong.
  if (!(m_calculator->Process(a_step, identifier, identifier_sr, vtmp, LArG4::kOnlyID))) {
    return false;
}
  if (identifier_sr != LArG4Identifier()) {
      SrHit(identifier, identifier_sr, a_energies);
    }
  else {
    // G4cout << "SpecialHit: No SR identifier provided." << G4endl;
  }

  return SimpleHit(identifier, a_energies, false);
} 

G4bool LArG4CalibSD::SrHit(const LArG4Identifier& a_ident, const LArG4Identifier& sr_id,
                           const std::vector<double>& energies)
{
  // Get particle ID (same as in SimpleHit)
  int particleID{HepMC::UNDEFINED_ID};
  int particleUID{HepMC::UNDEFINED_ID};
  if(m_doPID) {
    AtlasG4EventUserInfo* atlasG4EvtUserInfo = dynamic_cast<AtlasG4EventUserInfo*>(
    G4RunManager::GetRunManager()->GetCurrentEvent()->GetUserInformation());
    if(atlasG4EvtUserInfo) {
      particleID = HepMC::barcode(atlasG4EvtUserInfo->GetCurrentPrimaryGenParticle());
      particleUID = HepMC::uniqueID(atlasG4EvtUserInfo->GetCurrentPrimaryGenParticle());
    }
  }

  Identifier sr_id_converted = ConvertSRID(sr_id, a_ident);

  // Skip trivially small energy deposits (same as in SimpleHit, but with a looser threshold)
  if(energies[0]+energies[1]+energies[3] < 0.00001*CLHEP::eV && std::abs(energies[2]) < 0.00001*CLHEP::eV) {
    return true;
  }

  auto* calibrationHitsSr = getSrCalibrationHits();
  if (!calibrationHitsSr) {
    return false;
  }
  calibrationHitsSr->AddHit(m_hitSourceName,
                            std::make_unique<CaloCalibrationHit>(
                              sr_id_converted,
                              energies[0],
                              energies[1],
                              energies[2],
                              energies[3],
                              particleID,
                              particleUID));
  return true;
}


Identifier LArG4CalibSD::ConvertSRID(const LArG4Identifier& sr_id, const LArG4Identifier& lr_id) const
{
    // First convert the LR ID to a standard Identifier

    // G4cout << "Lr_id" << std::hex << lr_id[0] << std::dec << G4endl;
    Identifier base_id = ConvertID(lr_id);

    // Print the base ID for debugging
    // G4cout << "ConvertSRID: Base ID = " << std::hex << base_id.get_compact() << std::dec << G4endl;
    
    // If base conversion failed, we can't proceed
    if (!base_id.is_valid()){
      Identifier wrong_id = Identifier();
      wrong_id.set_literal(0xffffffffffffffff);
      return wrong_id;
    }
    
    // Create a value containing the SR ID in the lower 32 bits
    Identifier::value_type sr_value = static_cast<Identifier::value_type>(sr_id[0] & 0xFFFF);

    // G4cout << "ConvertSRID: SR ID = " << std::hex << sr_value << std::dec << G4endl;
    
    // Combine the base ID (upper 32 bits) with the SR ID (lower 32 bits)
    // First clear any lower 32 bits that might be present
    Identifier::value_type combined_value = (base_id.get_compact() & 0xFFFFFFFF00000000ULL) | sr_value;

    // Create a new identifier with the combined value
    Identifier result;

    result.set_literal(combined_value);
    

    // G4cout << "ConvertSRID: Result ID = " << std::hex << result2.get_compact() << std::dec << G4endl;
    // // Print the result
    // G4cout << "ConvertSRID: Combined ID = " << result.get_compact() << G4endl;
    return result;
}

LArCalibrationHitContainerBuilder* LArG4CalibSD::getCalibrationHits(bool deadMaterialHit) const
{
  auto* eventManager = G4EventManager::GetEventManager();
  if (!eventManager) {
    return nullptr;
  }
  auto* eventInfo =
    dynamic_cast<AtlasG4EventUserInfo*>(eventManager->GetUserInformation());
  if (!eventInfo) {
    return nullptr;
  }
  std::shared_ptr<HitCollectionMap> hitCollections = eventInfo->GetHitCollectionMap();
  if (!hitCollections) {
    return nullptr;
  }
  const std::string& collectionName =
    deadMaterialHit ? m_deadHitCollectionName : m_hitCollectionName;
  if (collectionName.empty()) {
    return nullptr;
  }
  return hitCollections->Find<LArCalibrationHitContainerBuilder>(collectionName);
}

LArSrCalibrationHitContainerBuilder* LArG4CalibSD::getSrCalibrationHits() const
{
  auto* eventManager = G4EventManager::GetEventManager();
  if (!eventManager) {
    return nullptr;
  }
  auto* eventInfo =
    dynamic_cast<AtlasG4EventUserInfo*>(eventManager->GetUserInformation());
  if (!eventInfo) {
    return nullptr;
  }
  std::shared_ptr<HitCollectionMap> hitCollections = eventInfo->GetHitCollectionMap();
  if (!hitCollections || m_srHitCollectionName.empty()) {
    return nullptr;
  }
  return hitCollections->Find<LArSrCalibrationHitContainerBuilder>(m_srHitCollectionName);
}

Identifier LArG4CalibSD::ConvertID(const LArG4Identifier& a_ident) const
{
    Identifier id;
    
    if(a_ident[0]==4)
    {
        // is LAr
        if(a_ident[1]==1)
        {
            //is LAr EM
            try
            {
                id = m_larEmID->channel_id(a_ident[2],  // barrel_ec
                                           a_ident[3],  // sampling
                                           a_ident[4],  // region
                                           a_ident[5],  // eta
                                           a_ident[6]); // phi
            }
            catch (LArID_Exception& e)
            {
                G4cout << "ERROR ConvertID: LArEM_ID error code " << e.code() << " "
                << (std::string) e << G4endl;
            }
        }
        else if(a_ident[1]==2)
        {
            //is EM HEC
            try
            {
                id = m_larHecID->channel_id(a_ident[2],  // zSide
                                            a_ident[3],  // sampling
                                            a_ident[4],  // region
                                            a_ident[5],  // eta
                                            a_ident[6]); // phi
            }
            catch(LArID_Exception& e)
            {
                G4cout << "ERROR ConvertID: BuildHitCollections: LArHEC_ID error code " << e.code() << " "
                << (std::string) e << G4endl;
            }
        }
        else if(a_ident[1]==3)
        {
            // FCAL
            if(a_ident[3]>0)
            {
                //is EM FCAL
                try
                {
                    id = m_larFcalID->channel_id(a_ident[2],  // zSide
                                                 a_ident[3],  // sampling
                                                 a_ident[4],  // eta
                                                 a_ident[5]); // phi
                }
                catch(LArID_Exception& e)
                {
                    G4cout << "ERROR ConvertID:: LArFCAL_ID error code " << e.code() << " "
                    << (std::string) e << G4endl;
                }
            }
            else
            {
              //is Mini FCAL
              G4cout << "ERROR ConvertID:: unsupported LArMiniFCAL Identifier " << G4endl;
            }
        }
    }
    else if(a_ident[0]==10)
    {
      // This is a dead-material identifier
      try 
      {
        id = m_caloDmID->zone_id(a_ident[1],  // zSide
                     a_ident[2],  // type
                     a_ident[3],  // sampling
                     a_ident[4],  // region
                     a_ident[5],  // eta
                     a_ident[6]); // phi
      }
      catch(CaloID_Exception& e)
      {
        G4cout << "ERROR ConvertID: CaloDM_ID error code " << e.code() << " "
               << (std::string) e << G4endl;
      }
    }

    return id;
}
